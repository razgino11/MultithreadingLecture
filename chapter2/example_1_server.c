#include <stdio.h> 
#include <string.h>   //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>   //close 
#include <arpa/inet.h>    //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include <pthread.h>
#include <signal.h>
     
#define TRUE   1 
#define FALSE  0 
#define PORT 8888 
#define MAX_CLIENTS 2000

void* connection_handler(void *arg);
void clean_string(char *str, int len);

pthread_mutex_t print_mutex;
struct sockaddr_in server;
int master_socket , addrlen;

// bind the signal exit and release the port if the program closes
void signal_handler(int sig)
{
    if (sig == SIGINT)
    {
        printf("\nCtrl+C pressed. Exiting...\n");
        close(master_socket);
        unlink("/tmp/socket");
        exit(0);
    }
}

int main(int argc , char *argv[])  
{  
    signal(SIGINT, signal_handler);
    if ((master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed\n");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    int reuse = 1;
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");
    // bind the socket to localhost port 8080
    if (bind(master_socket, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind failed\n");
        exit(EXIT_FAILURE);
    }
    
    //put the socket into listening mode with max of 10 pending connections
    if(listen(master_socket, 10) < 0)
    {
        perror("listen\n");
        exit(EXIT_FAILURE);
    }
    
    addrlen = sizeof(server);
    pthread_t threads[MAX_CLIENTS];
    int num_of_connections = 0;
    while (TRUE)
    {
        if (num_of_connections >= MAX_CLIENTS)
        {
            exit(EXIT_FAILURE);
        }
        struct sockaddr_in address;
        int new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        printf("accepted in main fd=%d\n", new_socket);
        if (new_socket < 0)
        {
            exit(EXIT_FAILURE);
        }
        pthread_t thread;
        printf("new connection\n");
        pthread_create(&thread, NULL, connection_handler, (void*) &new_socket);;     
        threads[num_of_connections] = thread;

        num_of_connections += 1;
        sleep(1);
    }


    for (int i=0; i<MAX_CLIENTS;i++)
    {
        pthread_join(threads[i], NULL);
    }
    
   return 0;
}  

char* removeTabsFromStr(char *string)
{
}

// replace invalid chars in the string
void clean_string(char *str, int len)
{
    int non_space_count = 0;
 
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] > 32 && str[i] < 126)
        {
            str[non_space_count] = str[i];
            non_space_count++;
        }    
    }

    str[non_space_count] = '\0';
}

void* connection_handler(void *arg)
{
    int socket_fd = *(int*)arg;

    // request the client's name
    char request_name[] = "Please enter your name: ";
    write(*(int*)arg, request_name, strlen(request_name));

    // receive it into the buffer
    char client_name[100] = {0};
    int n = recv(socket_fd, client_name, 100, 0);
    if (n < 0)
    {
        exit(EXIT_FAILURE);
    }
    
    // replace invalid chars in the name
    clean_string(client_name, n);
    client_name[n] = '\0';

    // print the client's name    
    printf("%s has joined the chat!\n", client_name);
    
    char buffer[1024] = {0};
    size_t valread = 0;
    // loop until the client disconnects and print their messages
    while (TRUE)
    {
        
        if ((valread = read( socket_fd , buffer, 1024)) > 0)
        {
            clean_string(buffer, valread);
            buffer[valread] = '\0';  
            printf("%s: %s\n", client_name, buffer);
        }

        sleep(0.1);
    }
}