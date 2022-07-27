#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <net/if.h>


#define PORT 8888

void broadcast_message();

int main(int argc, char const *argv[])
{
    // send a broadcast message using UDP
    broadcast_message();
    return 0;
}

void broadcast_message()
{
    // create a socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("Error creating socket!\n");
        return;
    }

    // create a socket address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_BROADCAST;
    // set specefic ip
    //server_addr.sin_addr.s_addr = inet_addr("172.17.0.0");
    socklen_t server_addr_len = sizeof(server_addr);

    // set the SO_BROADCAST option
    int broadcast = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
    {
        printf("Error setting socket option!\n");
        return;
    }
    //struct ifreq ifr;

    // send messages using the docker0 interface
    //memset(&ifr, 0, sizeof(ifr));
    //snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "docker0");

    // send a message
    char message[] = "Hello World!";
    int send_status = sendto(sockfd, message, sizeof(message), 0, (struct sockaddr *)&server_addr, server_addr_len);
    if (send_status < 0)
    {
        printf("Error sending message %d!\n", send_status);
        // print the errno
        perror("sendto");
        return;
    }
    printf("Message sent!\n");
}

