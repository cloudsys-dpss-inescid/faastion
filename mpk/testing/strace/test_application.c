#include <stdio.h>
#include <pthread.h>

void *
printHello(void *arg)
{
    printf("Hello from the new thread!\n");
    pthread_exit(NULL);
}

int 
main()
{
    pthread_t tid;
    printf("Hello from the main thread!\n");

    if (pthread_create(&tid, NULL, printHello, NULL) != 0)
    {
        printf("Error creating thread\n");
        return 1;
    }

    printf("Joining child\n");

    // Wait for the new thread to finish
    pthread_join(tid, NULL);

    printf("After child exited\n");

    return 0;
}
