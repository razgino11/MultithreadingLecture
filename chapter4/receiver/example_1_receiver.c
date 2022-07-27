#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8888
#define FALSE 0
#define TRUE 1

void recieve_broadcast_message();

int main(int argc, char const *argv[])
{
    // send a broadcast message using UDP
    recieve_broadcast_message();
    return 0;
}

void recieve_broadcast_message()
{    
    // receive a message using udp
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("Error creating socket!\n");
        return;
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    socklen_t server_addr_len = sizeof(server_addr);
    int bind_status = bind(sockfd, (struct sockaddr *)&server_addr, server_addr_len);
    if (bind_status < 0)
    {
        printf("Error binding socket!\n");
        printf("%d\n", bind_status);
        // print the errno
        perror("bind");
        return;
    }
    printf("listening");
    while (TRUE)
    {
        char message[1024];
        int recv_status = recvfrom(sockfd, message, sizeof(message), 0, (struct sockaddr *)&server_addr, &server_addr_len);
        if (recv_status < 0)
        {
            printf("Error receiving message %d!\n", recv_status);
            // print the errno
            perror("recvfrom");
            return;
        }

        printf("Message received: %s\n", message);
    }
}