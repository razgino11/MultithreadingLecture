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
     
#define TRUE   1 
#define FALSE  0 
#define PORT 8888 

void* connection_handler(void *arg);
     
int main(int argc , char *argv[])  
{  
    //create server that listen on 8080 
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
    char *message , client_message[2000];
    int opt = TRUE;
    int master_socket , addrlen , new_socket , max_clients = 6;
    int max_sd;
    struct timeval timeout;
    fd_set readfds;

    if ((master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed\n");
        exit(EXIT_FAILURE);
    }

    //type of socket created
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( PORT );

    //bind the socket to localhost port 8080
    if (bind(master_socket, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind failed\n");
        exit(EXIT_FAILURE);
    }

    //put the socket into listening mode
    if(listen(master_socket, 10) < 0)
    {
        perror("listen\n");
        exit(EXIT_FAILURE);
    }
    
    addrlen = sizeof(server);
    pthread_t threads[2000];
    int num_of_connections = 0;
    while (TRUE)
    {
        if (num_of_connections >= 2000)
        {
            exit(EXIT_FAILURE);
        }
        struct sockaddr_in address;
        printf("accepting\n");
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

    //accept the incoming connection
    puts("Waiting for connections ...\n");
    for (int i=0; i<2000;i++)
    {
        pthread_join(threads[i], NULL);
    }
    
   return 0;
}  


void* connection_handler(void *arg)
{
    int socket_fd = *(int*)arg;
    printf("accepted with fd=%d\n", socket_fd);
    while (TRUE)
    {
        char buffer[1024] = {0};
        size_t valread = 0;
        if ((valread = read( socket_fd , buffer, 1024)) > 0)
        {
            printf("recived %d bytes\n", valread);
            buffer[valread] = '\0';  
            printf("%s\n", buffer);
        }

        sleep(1);
    }
}