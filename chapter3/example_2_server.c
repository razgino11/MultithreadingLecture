#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>    //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/poll.h>

#define TRUE 1
#define FALSE 0
#define PORT 8888
#define MAX_CLIENTS 2000

void clean_string(char *str, int len);

struct sockaddr_in server;
int master_socket, addrlen;
int client_socket[MAX_CLIENTS];
int sd;
int max_sd;
int activity;
fd_set readfds;
int num_of_connections = 0;
struct timeval tv;
char buffer[1024];

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

int main(int argc, char *argv[])
{
    printf("Server is running, using Select! :)...\n");

    // bind the signal exit and release the port if the program closes
    signal(SIGINT, signal_handler);

    // create a master socket that will listen for incoming connections
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
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

    // bind the socket to localhost port 8888
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

    addrlen = sizeof(server);
    while (TRUE)
    {
        FD_ZERO(&readfds);

        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            // socket descriptor
            sd = client_socket[i];

            // if valid socket descriptor then add to read list
            if (sd > 0)
                FD_SET(sd, &readfds);

            // highest file descriptor number, need it for the select function
            if (sd > max_sd)
                max_sd = sd;
        }

        tv.tv_sec = 30;
        tv.tv_usec = 0;
        // wait for an activity on one of the sockets, timeout is tv;
        activity = select(max_sd + 1, &readfds, NULL, NULL, &tv);

        // return value < 0, error happend;
        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }
        // return value = 0, timeout;
        else if (activity == 0)
        {
            // no event happend in specific time, timeout;
            continue;
        }

        // If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            int new_socket;           

            if ((new_socket = accept(master_socket, (struct sockaddr *)&server, (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // inform user of socket number - used in send and receive commands
            printf("New connection from ip %s and port %d \n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));

            // add new socket to array of sockets
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                // if position is empty
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    printf("New Client: Client %d\n", i);
                    break;
                }
            }
        }

        // else there is some IO operation on some other socket :)
        for (int i = 0; i < max_sd; i++)
        {
            sd = client_socket[i];
            if (FD_ISSET(sd, &readfds))
            {
                // Check if it was for closing , and also read the incoming message
                int valread = read(sd, buffer, 1024);
                // Echo back the message that came in
                if (valread > 0)
                {
                    clean_string(buffer, strlen(buffer));
                    printf("client %d: %s\n", i, buffer);
                    memset(buffer, 0, valread);
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
