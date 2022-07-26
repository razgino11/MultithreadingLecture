#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void *threaded_function(void *arg);

int global_var = 0;
pthread_mutex_t mutex;

int main(int argc, char const *argv[])
{
    // create 2 threads
    pthread_t thread_1, thread_2;
    int thread_1_status, thread_2_status;
    thread_1_status = pthread_create(&thread_1, NULL, threaded_function, NULL);
    thread_2_status = pthread_create(&thread_2, NULL, threaded_function, NULL);
    pthread_join(thread_1, NULL);
    pthread_join(thread_2, NULL);

    printf("global_var = %d\n", global_var);
    return 0;
}

void *threaded_function(void *arg)
{
    for (int i = 0; i < 100000; i++)
    {
        pthread_mutex_lock(&mutex);
        global_var++;
        pthread_mutex_unlock(&mutex);
    }
}
