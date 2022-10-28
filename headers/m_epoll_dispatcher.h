#ifndef M_EPOLL_DISPATCHER_H
#define M_EPOLL_DISPATCHER_H
#include <sys/epoll.h>
#include "m_event_dispatcher.h"

extern "C"
{
    #include "../lib/log.h"
}

#define MAXEVENTS 128

struct epoll_dispatcher_data 
{
    int event_count;
    int nfds;
    int realloc_copy;
    int efd;
    struct epoll_event* events;

    ~epoll_dispatcher_data()
    {
        if(events != nullptr)
            delete events;
    }
};

class EpollDispatcher : public EventDispatcher
{
public:
    EpollDispatcher();

    ~EpollDispatcher();

    epoll_dispatcher_data get_data()
    {
        return m_data;
    }

    /** 通知dispatcher新增一个channel事件*/
    int add(EventLoop* eventLoop, Channel* channel);

    /** 通知dispatcher删除一个channel事件*/
    int del(EventLoop* eventLoop, Channel* channel);

    /** 通知dispatcher更新channel对应的事件*/
    int update(EventLoop* eventLoop, Channel* channel);

    /** 实现事件分发，然后调用event_loop的event_activate方法执行callback*/
    int dispatch(EventLoop* eventLoop, struct timeval*);

    /** 清除数据 */
    void clear(EventLoop* eventLoop);

private:
    epoll_dispatcher_data m_data;
};
#endif