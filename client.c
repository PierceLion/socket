
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    int sockFd, addrLen, result;
    struct sockaddr_in address;
    char send_buffer[128];
    bzero(send_buffer, sizeof(send_buffer));

    //build client socket
    sockFd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(9734);
    addrLen = sizeof(address);

    //no wait time to reuse the socket
    int bReuseaddr = 1;
    setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&bReuseaddr, sizeof(bReuseaddr));

    result = connect( sockFd, (struct sockaddr *) &address, addrLen);
    if(result == -1)
    {
        perror("Oooops: client");
        return (-1);
    }

    while(1)
    {
        scanf("%s", send_buffer);
        if(send(sockFd, &send_buffer, sizeof(send_buffer), 0) < 0)
        {
            perror("client send");
            return (-1);
        }
        bzero(send_buffer, sizeof(send_buffer));
        if(recv(sockFd, &send_buffer, sizeof(send_buffer), 0) < 0)
        {
            perror("client recv");
            return (-1);
        }
        printf("char from the server = %s\n", send_buffer);
    }
    close(sockFd);
    return 0;
}
