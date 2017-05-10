#include "utils/wtime.h"
#include <stdio.h>

/* Implementation */

uint32_t get_stime(void)
{
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

int64_t get_ustime(void) {
    timeval tv;
    int64_t ust;

    gettimeofday(&tv, NULL);
    ust = ((int64_t)tv.tv_sec) * 1000000;
    ust += tv.tv_usec;
    return ust;
}

int64_t get_mstime(void) {
    timeval tv;
    int64_t mst;

    gettimeofday(&tv, NULL);
    mst = ((int64_t)tv.tv_sec) * 1000;
    mst += tv.tv_usec / 1000;
    return mst;
}

void get_time_pair(uint32_t& s, uint32_t& m)
{
    timeval tv;

    gettimeofday(&tv, NULL);
    s = tv.tv_sec;
    m = (uint32_t)tv.tv_usec / 1000;
}
