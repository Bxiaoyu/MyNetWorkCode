#include "m_poll_dispatcher.h"

PollDispatcher::PollDispatcher()
{
    m_data.event_set = new pollfd[INIT_POLL_SIZE];

    for(int i = 0; i < INIT_POLL_SIZE; i++)
    {
        m_data.event_set[i].fd = -1;
    }
    m_data.event_count = 0;
    m_data.nfds = 0;
    m_data.realloc_copy = 0;
}

PollDispatcher::~PollDispatcher()
{
    if(m_data.event_set != nullptr)
    {
        delete[] m_data.event_set;
        m_data.event_set = nullptr;
    }
    if(m_data.event_set_copy != nullptr)
    {
        delete[] m_data.event_set_copy;
        m_data.event_set_copy = nullptr;
    }
}

/** 通知dispatcher新增一个channel事件*/
int PollDispatcher::add(EventLoop* eventLoop, Channel* channel)
{}

/** 通知dispatcher删除一个channel事件*/
int PollDispatcher::del(EventLoop* eventLoop, Channel* channel)
{}

/** 通知dispatcher更新channel对应的事件*/
int PollDispatcher::update(EventLoop* eventLoop, Channel* channel)
{}

/** 实现事件分发，然后调用event_loop的event_activate方法执行callback*/
int PollDispatcher::dispatch(EventLoop* eventLoop, struct timeval*)
{}

/** 清除数据 */
void PollDispatcher::clear(EventLoop* eventLoop)
{}