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

void* send_handler(void *arg);
void* receive_handler(void *arg);

void clean_string(char *str, int len);

pthread_mutex_t print_mutex;
struct sockaddr_in server;
int master_socket, addrlen;

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
    // bind the signal exit and release the port if the program closes
    signal(SIGINT, signal_handler);

    // create a master socket that will listen for incoming connections
    if ((master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed\n");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // setting reuseaddr to true to avoid the error message "Address already in use"
    int reuse = 1;
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    // bind the socket to localhost port 8080
    if (bind(master_socket, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind failed\n");
        exit(EXIT_FAILURE);
    }
    
    // put the socket into listening mode with max of 10 pending connections
    if (listen(master_socket, 10) < 0)
    {
        perror("listen\n");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for a connection...\n");
    struct sockaddr_in address;
    int new_socket = accept(master_socket, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    printf("Accepted new connection with fd=%d\n", new_socket);
    
    pthread_t receive_thread, send_thread;
    // use the 2 threads
    pthread_create(&receive_thread, NULL, receive_handler, (void*)&new_socket);
    pthread_create(&send_thread, NULL, send_handler, (void*)&new_socket);
    pthread_join(receive_thread, NULL);
    pthread_join(send_thread, NULL);
    
    return 0;  
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

void* send_handler(void *arg)
{
    int socket_fd = *(int*)arg;
    
    char buffer[1024] = {0};
    // loop until the client disconnects and print their messages
    while (TRUE)
    {
        // accept a string from console
        printf("> ");
        fgets(buffer, 1024, stdin);

        // send the string to the client
        if (strlen(buffer) > 0)
        {
            send(socket_fd, buffer, strlen(buffer), 0);
        }
        
        sleep(0.1);
    }
}

void* receive_handler(void *arg)
{
    int socket_fd = *(int*)arg;
    
    char buffer[1024] = {0};
    int valread = 0;
    // loop until the client disconnects and print their messages
    while (TRUE)
    {
        if ((valread = read(socket_fd , buffer, 1024)) > 0)
        {
            clean_string(buffer, valread);
            buffer[valread] = '\0';  
            printf("\rClient: %s\n> ", buffer);
            fflush(stdout);
        }

        sleep(0.1);
    }
}