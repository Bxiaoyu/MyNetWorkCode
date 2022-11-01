#include "m_channel.h"
#include "m_event_loop.h"

Channel::Channel(int fd, int events, event_read_callback eventReadCallback, event_write_callback eventWriteCallback, void* data)
{
    this->channel_init(fd, events, eventReadCallback, eventWriteCallback, data);
}

Channel::~Channel()
{
    
}

void Channel::channel_init(int fd, int events, event_read_callback eventReadCallback, event_write_callback eventWriteCallback, void* data)
{
    this->fd = fd;
    this->events = events;
    this->eventReadCallback = eventReadCallback;
    this->eventWriteCallback = eventWriteCallback;
    this->data = data;
}

int Channel::write_event_is_enabled()
{
    return this->events & EVENT_WRITE;
}

int Channel::write_event_enable()
{
    EventLoop* loop = (EventLoop*)this->data;
    events = events | EVENT_WRITE;
    loop->event_loop_update_channel_event(fd, this);
}

int Channel::write_event_disable()
{
    EventLoop* loop = (EventLoop*)this->data;
    events = events & ~EVENT_WRITE;
    loop->event_loop_update_channel_event(fd, this);
}