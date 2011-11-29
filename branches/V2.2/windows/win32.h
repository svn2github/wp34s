#pragma once

struct timeval 
{
        long    tv_sec;         /* seconds */
        long    tv_usec;        /* and microseconds */
};

int gettimeofday(struct timeval *tv, struct timezone *tz);


