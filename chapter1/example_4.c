#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void *infinite_thread(void *arg);

int join_thread = 1;

int main(int argc, char const *argv[])
{
    // create an infinite printing thread
    pthread_t thread_1;
    int thread_1_status;
    thread_1_status = pthread_create(&thread_1, NULL, infinite_thread, NULL);

    // if we dont join the thread, the program will exit
    if (join_thread)
    {
        pthread_join(thread_1, NULL);
    }

    printf("Program is done!\n");
    return 0;
}

void *infinite_thread(void *arg)
{
    while (1)
    {
        printf("Hello Infinite Thread!\n");
        sleep(1);
    }
}