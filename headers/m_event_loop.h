#ifndef M_EVENT_LOOP_H
#define M_EVENT_LOOP_H
#include <pthread.h>
#include "m_channel.h"
#include "m_channel_map.h"
#include "m_event_dispatcher.h"
#include "m_epoll_dispatcher.h"
#include "m_poll_dispatcher.h"

extern "C"
{
    #include "../lib/common.h"
    #include "../lib/log.h"
}

struct channel_element
{
    int type;  //1: add  2: delete
    Channel* channel;
    channel_element* next;
};

class EventLoop
{
public:
    EventLoop(char* name = nullptr);

    ~EventLoop();

    int event_loop_run();

    int event_loop_add_channel_event(int fd, Channel* channel1);

    int event_loop_remove_channel_event(int fd, Channel* channel1);

    int event_loop_update_channel_event(int fd, Channel* channel1);

    // dispather派发完事件之后，调用该方法通知event_loop执行对应事件的相关callback方法
    // res: EVENT_READ | EVENT_READ等
    int channel_event_activate(int fd, int res);

    int handleWakeup(void *data);

    pthread_t get_owner_thread_id() const
    {
        return owner_thread_id;
    }

private:
    void event_loop_wakeup();

    int event_loop_do_channel_event(int fd, Channel *channel1, int type);

    // in the i/o thread
    int event_loop_handle_pending_channel();

    void event_loop_channel_buffer_nolock(int fd, Channel* channel1, int type);

    // in the i/o thread
    int event_loop_handle_pending_add(int fd, Channel* channel);

    // in the i/o thread
    int event_loop_handle_pending_remove(int fd, Channel* channel);

    // in the i/o thread
    int event_loop_handle_pending_update(int fd, Channel* channel);

private:
    int quit;
    EventDispatcher* eventDispatcher;

    /** 对应的event_dispatcher的数据. */
    void* event_dispatcher_data;
    Channel_map* channelMap;

    int is_handle_pending;
    channel_element* pending_head;
    channel_element* pending_tail;

    pthread_t owner_thread_id;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int socketPair[2];
    char* thread_name;
};

#endif