#ifndef M_EVENT_DISPATCHER_H
#define M_EVENT_DISPATCHER_H
#include "m_channel.h"

class EventLoop;

class EventDispatcher
{
public:
    /** 通知dispatcher新增一个channel事件*/
    virtual int add(EventLoop* eventLoop, Channel* channel)=0;

    /** 通知dispatcher删除一个channel事件*/
    virtual int del(EventLoop* eventLoop, Channel* channel)=0;

    /** 通知dispatcher更新channel对应的事件*/
    virtual int update(EventLoop* eventLoop, Channel* channel)=0;

    /** 实现事件分发，然后调用event_loop的event_activate方法执行callback*/
    virtual int dispatch(EventLoop* eventLoop, struct timeval*)=0;

    /** 清除数据 */
    virtual void clear(EventLoop* eventLoop)=0;

    /** 返回数据 */
    virtual void* get_data()=0;

public:
    char* name;  /**  对应实现 */
};

#endif