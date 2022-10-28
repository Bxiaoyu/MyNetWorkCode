#include "m_channel.h"

Channel::Channel(int fd, int events, event_read_callback eventReadCallback, event_write_callback eventWriteCallback, void* data)
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

}

int Channel::write_event_disable()
{

}