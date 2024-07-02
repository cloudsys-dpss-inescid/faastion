#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>

#define MAXTHREADS 1000000
//#define THREADSTACK  65536
#define THREADSTACK  32768

static unsigned long long thread_nr = 0;

pthread_mutex_t mutex_;

void printRlimit(const char *msg, int resource){
    struct rlimit rlim;
    getrlimit(resource, &rlim);
    printf("%s ", msg);

    printf("soft=");
    if (rlim.rlim_cur == RLIM_INFINITY) {
        printf("infinite");
    }
    else if (rlim.rlim_cur == RLIM_SAVED_CUR) {
        printf("unrepresentable");
    }
    else {
        printf("%lld", (long long) rlim.rlim_cur);
    }

    printf(" hard=");
    if (rlim.rlim_max == RLIM_INFINITY) {
        printf("infinite\n");
    }
    else if (rlim.rlim_max == RLIM_SAVED_MAX) {
        printf("unrepresentable");
    }
    else {
       printf("%lld\n", (long long) rlim.rlim_max);
    }
}

void printLimits() {
    printRlimit("RLIMIT_AS", RLIMIT_STACK);
    printRlimit("RLIMIT_CORE", RLIMIT_CORE);
    printRlimit("RLIMIT_CPU", RLIMIT_CPU);
    printRlimit("RLIMIT_DATA", RLIMIT_DATA);
    printRlimit("RLIMIT_FSIZE", RLIMIT_FSIZE);
    printRlimit("RLIMIT_MEMLOCK", RLIMIT_MEMLOCK);
    printRlimit("RLIMIT_MSGQUEUE", RLIMIT_MSGQUEUE);
    printRlimit("RLIMIT_NPROC", RLIMIT_NPROC);
    printRlimit("RLIMIT_NICE", RLIMIT_NICE);
    printRlimit("RLIMIT_NOFILE", RLIMIT_NOFILE);
    printRlimit("RLIMIT_RSS", RLIMIT_RSS);
    printRlimit("RLIMIT_RTPRIO", RLIMIT_RTPRIO);
    printRlimit("RLIMIT_RTTIME", RLIMIT_RTTIME);
    printRlimit("RLIMIT_SIGPENDING", RLIMIT_SIGPENDING);
    printRlimit("RLIMIT_STACK", RLIMIT_STACK);
}

void* inc_thread_nr(void* arg) {
    (void*)arg;
    pthread_mutex_lock(&mutex_);
    thread_nr ++;
    pthread_mutex_unlock(&mutex_);

    printf("thread_nr = %d\n", thread_nr);

    sleep(300000);
}

int main(int argc, char *argv[])
{
    pthread_t pid[MAXTHREADS];
    pthread_attr_t attrs;
    int err, i;
    int cnt = 0;

    printLimits();

    pthread_attr_init(&attrs);
    pthread_attr_setstacksize(&attrs, THREADSTACK);
    pthread_mutex_init(&mutex_, NULL);

    for (cnt = 0; cnt < MAXTHREADS; cnt++) {
        err = pthread_create(&pid[cnt], &attrs, (void*)inc_thread_nr, NULL);
        if (err != 0) {
	    fprintf(stderr, "Failed with error %d: ", err);
            perror(NULL);
            break;
        }
    }

    pthread_attr_destroy(&attrs);

    for (i = 0; i < cnt; i++) {
        pthread_join(pid[i], NULL);
    }

    pthread_mutex_destroy(&mutex_);

    printf("Maximum number of threads per process is %d (%d)\n", cnt, thread_nr);
}
