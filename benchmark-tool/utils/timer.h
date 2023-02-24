#include <sys/time.h>

#define TIMER struct timespec

TIMER read_time(TIMER time) {
    if (clock_gettime(CLOCK_REALTIME, &time) == -1) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    } else {
       return time;
    }
}                

long time_diff(TIMER start, TIMER stop){
    return ((double)stop.tv_sec - (double)start.tv_sec) * 1000000000 + ((double)stop.tv_nsec - (double)start.tv_nsec);
}
