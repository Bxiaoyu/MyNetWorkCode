#include "m_epoll_dispatcher.h"

EpollDispatcher::EpollDispatcher()
{
    m_data.event_count = 0;
    m_data.nfds = 0;
    m_data.realloc_copy = 0;
    m_data.efd = 0;

    m_data.efd = epoll_create1(0);
    if(m_data.efd == -1)
    {
        error(1, errno, "epoll create failed.");
    }

    m_data.events = new epoll_event[MAXEVENTS];
}

EpollDispatcher::~EpollDispatcher()
{
    if(m_data.events != nullptr)
    {
        delete[] m_data.events;
        m_data.events = nullptr;
    }   
}

/** 通知dispatcher新增一个channel事件*/
int EpollDispatcher::add(EventLoop* eventLoop, Channel* channel)
{
}

/** 通知dispatcher删除一个channel事件*/
int EpollDispatcher::del(EventLoop* eventLoop, Channel* channel)
{}

/** 通知dispatcher更新channel对应的事件*/
int EpollDispatcher::update(EventLoop* eventLoop, Channel* channel)
{}

/** 实现事件分发，然后调用event_loop的event_activate方法执行callback*/
int EpollDispatcher::dispatch(EventLoop* eventLoop, struct timeval*)
{}

/** 清除数据 */
void EpollDispatcher::clear(EventLoop* eventLoop)
{}
