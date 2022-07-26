#include <pthread.h>
#include <stdio.h>

void *thread_1_function(void *arg);
void *thread_2_function(void *arg);

FILE *fp;
int main(int argc, char const *argv[])
{
    // open file
    fp = fopen("example_1.txt", "w");
    if (fp == NULL)
    {
        printf("Error opening file!\n");
        return -1;
    }

    // create 2 threads
    pthread_t thread_1, thread_2;
    int thread_1_status, thread_2_status;
    thread_1_status = pthread_create(&thread_1, NULL, thread_1_function, NULL);
    thread_2_status = pthread_create(&thread_2, NULL, thread_2_function, NULL);
    pthread_join(thread_1, NULL);
    pthread_join(thread_2, NULL);

    fclose(fp);
    return 0;
}

void *thread_1_function(void *arg)
{
    // write to file
    fprintf(fp, "Thread 1: Hello Thread 1!\n");
    printf("Thread 1 hello world\n");
}

void *thread_2_function(void *arg)
{
    fprintf(fp, "Thread 2: Hello Thread 2!\n");
    printf("Thread 2 hello world\n");
}
