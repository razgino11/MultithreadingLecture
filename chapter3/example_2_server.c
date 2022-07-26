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
#include <sys/types.h>
#include <sys/poll.h>
     
#define TRUE   1 
#define FALSE  0 
#define PORT 8888 
#define MAX_CLIENTS 2000

void clean_string(char *str, int len);

pthread_mutex_t print_mutex;
struct sockaddr_in server;
int master_socket, addrlen;
struct pollfd fds[MAX_CLIENTS];
int num_of_connections = 0;

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
    // initialize poll values fd
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        fds[i].fd = -1;
        fds[i].events = 0;
    }
    
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
    // add the listen socket so that we can poll for incoming connections
    fds[num_of_connections].fd = master_socket;
    fds[num_of_connections].events = POLLIN;
    num_of_connections += 1;
    
    addrlen = sizeof(server);
    while (TRUE)
    {        
        // use select to select the correct fd
        int ret = select(num_of_connections, &fds[0], NULL, NULL, NULL);

        if (ret <= 0)
        {
            perror("select error");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < num_of_connections; i++)
        {
            // the first index in the array is for accepting new connections
            if (i == 0 && (fds[0].revents & POLLIN))
            {
                struct sockaddr_in address;
                int new_socket = accept(master_socket, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                printf("Accepted new connection with fd=%d\n", new_socket);

                if (new_socket < 0)
                {
                    exit(EXIT_FAILURE);
                }

                fds[num_of_connections].fd = new_socket;
                fds[num_of_connections].events = POLLRDNORM;
                num_of_connections += 1;
                continue;
            }
            if (i != 0 && (fds[i].revents & (POLLRDNORM | POLLERR)))
            {   
                // read the message from the client
                char buffer[1024] = {0};
                int bytes_read = read(fds[i].fd, buffer, 1024);
                if (bytes_read > 0)
                {
                    clean_string(buffer, strlen(buffer));
                    printf("client: %s\n", buffer);
                }
            }
        }
    }
    
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
