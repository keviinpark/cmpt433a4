// Manage time/sleep. Modified code provided by assignment description.

#include <time.h>

#include "common/timing.h"

#define MS_PER_SECOND 1000
#define NS_PER_MS 1000000
#define NS_PER_SECOND 1000000000

long long Timing_getTimeMS(void)
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoseconds = spec.tv_nsec;
    long long milliseconds = seconds * MS_PER_SECOND
        + nanoseconds / NS_PER_MS;
    return milliseconds;
}

void Timing_sleepForMS(long long delayMS)
{
    long long delayNS = delayMS * NS_PER_MS;
    int seconds = delayNS / NS_PER_SECOND;
    int nanoseconds = delayNS % NS_PER_SECOND;
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *) NULL);
}

