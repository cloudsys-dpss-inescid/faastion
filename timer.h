#include <sys/time.h>

#define TIMER struct timeval

TIMER read_time(TIMER time) {
    if(gettimeofday(&(time), NULL)){perror("gettimeofday failed"); exit(EXIT_FAILURE);}
    return time;
}                

float time_diff(TIMER start, TIMER stop){
    return (((double)(stop.tv_sec)  + (double)(stop.tv_usec / 1000000.0)) - ((double)(start.tv_sec) + (double)(start.tv_usec / 1000000.0)));
}