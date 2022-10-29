#include <cassert>
#include "m_event_loop.h"
#include "m_utils.h"

EventLoop::EventLoop(char* name)
{
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    if(name != nullptr)
    {
        thread_name = name;
    }
    else
    {
        thread_name = "main thread";
    }

    quit = 0;
    channelMap = new Channel_map;

#ifdef EPOLL_ENABLE
    yolanda_msgx("set epoll as dispatcher, %s", thread_name);
    eventDispatcher = (EventDispatcher*)new EpollDispatcher;
#else
    yolanda_msgx("set poll as dispatcher, %s", thread_name);
    eventDispatcher = (EventDispatcher*)new PollDispatcher;
#endif
    event_dispatcher_data = eventDispatcher->get_data();

    // add the socketfd to event
    owner_thread_id = pthread_self();
    if(socketpair(AF_UNIX, SOCK_STREAM, 0, socketPair) < 0)
    {
        LOG_ERR("socketpair set fialed");
    }
    is_handle_pending = 0;
    pending_head = nullptr;
    pending_tail = nullptr;

    // Channel chan(socketPair[1], EVENT_READ, handleWakeup, nullptr, (void*)this);
}

EventLoop::~EventLoop()
{}

int EventLoop::event_loop_handle_pending_channel()
{
    // Get the lock
    pthread_mutex_lock(&mutex);
    is_handle_pending = 1;

    channel_element* channelElement = pending_head;
    while(channelElement != nullptr)
    {
        // save into event_map
        Channel* chan = channelElement->channel;
        int fd = chan->get_fd();
        if(channelElement->type == 1)
        {
            event_loop_handle_pending_add(fd, chan);
        }
        else if(channelElement->type == 2)
        {
            event_loop_handle_pending_remove(fd, chan);
        }
        else if(channelElement->type == 3)
        {
            event_loop_handle_pending_update(fd, chan);
        }
        channelElement = channelElement->next;
    }

    pending_head = pending_tail = nullptr;
    is_handle_pending = 0;

    // Release the lock
    pthread_mutex_unlock(&mutex);

    return 0;
}

void EventLoop::event_loop_channel_buffer_nolock(int fd, Channel* channel1, int type)
{
    channel_element* channelElement = new channel_element;
    channelElement->channel = channel1;
    channelElement->type = type;
    channelElement->next = nullptr;

    // 第一个元素
    if(pending_head == nullptr)
    {
        pending_head = pending_tail = channelElement;
    }
    else
    {
        pending_tail->next = channelElement;
        pending_tail = channelElement;
    }
}

int EventLoop::event_loop_do_channel_event(int fd, Channel *channel1, int type)
{
    // get the lock
    pthread_mutex_lock(&mutex);
    assert(is_handle_pending == 0);
    event_loop_channel_buffer_nolock(fd, channel1, type);
    // release the lock
    pthread_mutex_unlock(&mutex);
    if(!isInSameThread(this))
    {
        event_loop_wakeup();
    }
    else
    {
        event_loop_handle_pending_channel();
    }

    return 0;
}

int EventLoop::event_loop_add_channel_event(int fd, Channel* channel1)
{
    return event_loop_do_channel_event(fd, channel1, 1);
}

int EventLoop::event_loop_remove_channel_event(int fd, Channel* channel1)
{
    return event_loop_do_channel_event(fd, channel1, 2);
}

int EventLoop::event_loop_update_channel_event(int fd, Channel* channel1)
{
    return event_loop_do_channel_event(fd, channel1, 3);
}

// in the i/o thread
int EventLoop::event_loop_handle_pending_add(int fd, Channel* channel)
{
    yolanda_msgx("add channel fd = %d, %s", fd, thread_name);
    Channel_map* map = channelMap;

    if(fd < 0)
    {
        return 0;
    }

    if(fd >= map->nentries)
    {
        if(map->map_make_space(fd, sizeof(Channel*)) == -1)
            return -1;
    }

    // 第一次创建，增加
    if((map)->entries[fd] == nullptr)
    {
        map->entries[fd] = channel;
        // add channel
        EventDispatcher* dispatcher = eventDispatcher;
        dispatcher->add(this, channel);
        return 1;
    }

    return 0;
}

// in the i/o thread
int EventLoop::event_loop_handle_pending_remove(int fd, Channel* channel)
{
    Channel_map* map = channelMap;
    assert(fd == channel->get_fd());

    if(fd < 0)
        return 0;
    
    if(fd >= map->nentries)
        return (-1);
    
    Channel* channel2 = (Channel*)map->entries[fd];

    //update dispatcher(multi-thread)here
    EventDispatcher* dispatcher = eventDispatcher;

    int retval = 0;
    if(dispatcher->del(this, channel2) == -1)
    {
        retval = -1;
    }
    else
    {
        retval = 1;
    }

    map->entries[fd] = nullptr;
    return retval;
}

// in the i/o thread
int EventLoop::event_loop_handle_pending_update(int fd, Channel* channel)
{
    yolanda_msgx("update channel fd == %d, %s", fd, thread_name);
    Channel_map* map = channelMap;

    if(fd < 0)
        return 0;
    
    if((map)->entries[fd] == nullptr)
        return (-1);
    
    // update channel
    EventDispatcher* dispatcher = eventDispatcher;
    dispatcher->update(this, channel);
}

// dispather派发完事件之后，调用该方法通知event_loop执行对应事件的相关callback方法
// res: EVENT_READ | EVENT_READ等
int EventLoop::channel_event_activate(int fd, int res)
{
    Channel_map* map = channelMap;
    yolanda_msgx("activate channel fd == %d, revents=%d, %s", fd, res, thread_name);

    if(fd < 0)
        return 0;
    
    if(fd >= map->nentries)
        return -1;

    Channel* channel = (Channel*)map->entries[fd];
    assert(fd == channel->get_fd());

    if(res & (EVENT_READ))
    {
        if(channel->eventReadCallback)
            channel->eventReadCallback(channel->data);
    }
    if(res & (EVENT_WRITE))
    {
        if(channel->eventWriteCallback)
            channel->eventWriteCallback(channel->data);
    }

    return 0;
}

void EventLoop::event_loop_wakeup()
{
    char one = 'a';
    ssize_t n = write(socketPair[0], &one, sizeof(one));
    if(n != sizeof(one))
    {
        LOG_ERR("wakeup event loop thread failed");
    }
}

int EventLoop::handleWakeup(void *data)
{
    EventLoop* eventLoop = (EventLoop*)data;
    char one;
    ssize_t n = read(socketPair[1], &one, sizeof(one));
    if(n != sizeof(one))
    {
        LOG_ERR("handleWakeup  failed");
    }

    yolanda_msgx("wakeup, %s", eventLoop->thread_name);
}

/**
 *
 * 1.参数验证
 * 2.调用dispatcher来进行事件分发,分发完回调事件处理函数
 */
int EventLoop::event_loop_run()
{
    assert(this != nullptr);

    EventDispatcher* dispatcher = eventDispatcher;

    if(owner_thread_id != pthread_self())
        exit(1);

    yolanda_msgx("event loop run, %s", thread_name);
    struct timeval tv;
    tv.tv_sec = 1;

    while(!quit)
    {
        //block here to wait I/O event, and get active channels
        dispatcher->dispatch(this, &tv);

        //handle the pending channel
        event_loop_handle_pending_channel();
    }

    yolanda_msgx("event loop end, %s", thread_name);
    return 0;
}