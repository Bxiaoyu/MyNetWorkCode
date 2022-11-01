#ifndef M_CHANNEL_H
#define M_CHANNEL_H
#include <iostream>
#include <functional>
#include "buffer.h"

#define EVENT_TIMEOUT    0x01
/** Wait for a socket or FD to become readable */
#define EVENT_READ        0x02
/** Wait for a socket or FD to become writeable */
#define EVENT_WRITE    0x04
/** Wait for a POSIX signal to be raised*/
#define EVENT_SIGNAL    0x08

// typedef int (*event_read_callback)(void* data);
// typedef int (*event_write_callback)(void* data);

class Channel
{
public:
    typedef int (*event_read_callback)(void* data);
    typedef int (*event_write_callback)(void* data);

    Channel(int fd, int events, event_read_callback eventReadCallback, event_write_callback eventWriteCallback, void* data);

    ~Channel();

    int write_event_is_enabled();

    int write_event_enable();

    int write_event_disable();

    int get_fd()
    {
        return fd;
    }

    void set_fd(int nfd)
    {
        fd = nfd;
    }

    int get_events()
    {
        return events;
    }

    void set_events(int event)
    {
        events = event;
    }

private:
    void channel_init(int fd, int events, event_read_callback eventReadCallback, event_write_callback eventWriteCallback, void* data);

private:
    int fd;
    int events;  // 表示事件类型

public:
    event_read_callback eventReadCallback;
    event_write_callback eventWriteCallback;

    void* data;  //callback data, 可能是event_loop，也可能是tcp_server或者tcp_connection
};
#endif