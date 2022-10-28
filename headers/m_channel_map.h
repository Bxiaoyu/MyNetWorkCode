#ifndef M_CHANNEL_MAP_H
#define M_CHANNEL_MAP_H
#include "m_channel.h"

/**
 * channel映射表, key为对应的socket描述字
 */
struct Channel_map
{
public:
    void** entries;

    /* The number of entries available in entries */
    int nentries;

public:
    Channel_map()
    {
        nentries = 0;
        entries = nullptr;
    }

    ~Channel_map()
    {
        if(entries != nullptr)
        {
            delete[] entries;
        }
        entries = nullptr;
        nentries = 0;
    }

    int map_make_space(int slot, int msize)
    {
        if(nentries <= slot)
        {
            int n = nentries ? nentries : 32;
            void** tmp;

            while(nentries <= slot)
                nentries <<= 2;
            
            tmp = (void**)realloc(entries, nentries * msize);
            if (tmp == NULL)
            {
                return (-1);
            }

            memset(&tmp[nentries], 0, (n - nentries) * msize);
            nentries = n;
            entries = tmp;
        }

        return (0);
    }
};
#endif