#include "m_utils.h"
#include "m_event_loop.h"

extern "C"
{
    #include "../lib/log.h"
}

void assertInSameThread(EventLoop *eventLoop)
{
    if(eventLoop->get_owner_thread_id() != pthread_self())
    {
        LOG_ERR("not in the same thread");
        exit(-1);
    }
}

//1： same thread: 0： not the same thread
int isInSameThread(EventLoop *eventLoop)
{
    return eventLoop->get_owner_thread_id() == pthread_self();
}
