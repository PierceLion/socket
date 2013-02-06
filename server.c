#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <pthread.h>

#define MAX_CLIENT 3

/* thread to handle the client fd */
void *thread_function(void *arg) {
    char buffer[128];
    int fd = (int)arg;

    while(1) {
        bzero(buffer, sizeof(buffer));
        int recv_num = recv(fd, &buffer, sizeof(buffer), 0);
        //recv error
        if( recv_num < 0)
        {
            perror("recv ");
            break;
        }
        //client disconnect
        else if(recv_num == 0)
        {
            printf("client fd %d disconnect\n", fd);
            break;
        }
        printf("client fd %d\n", fd);
        buffer[0]++;
        if(send(fd, &buffer, sizeof(buffer), 0) < 0)
        {
            perror("send ");
            break;
        }
    }

    pthread_exit("");
}

int main() {
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    int result;
    fd_set readfds, testfds;

    pthread_t thread_a;

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET; // TCP/IP
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // client IP constrant
    server_address.sin_port = htons(9734); //port
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

    listen(server_sockfd, 5);

    FD_ZERO(&readfds);
    FD_SET(server_sockfd, &readfds);

    //signal(SIGCHLD, SIG_IGN); //interupt
    printf("server waiting\n");
    while(1) {
        int fd;

        testfds = readfds;
        result = select(FD_SETSIZE, &testfds, (fd_set *)0, (fd_set *)0, (struct timeval *) 0);

        if(result < 1) {
            perror("server error");
            return (-1);
        }

        for(fd = 0; fd < FD_SETSIZE; fd++) {
            // some data
            if(FD_ISSET(fd, &testfds)) {
                //new connection
                if(fd == server_sockfd) {
                    client_len = sizeof(client_address);
                    client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
                    if(client_sockfd < 0)
                    {
                        perror("accept");
                        return (-1);
                    }

                    FD_SET(client_sockfd, &readfds);

                    printf("add client fd %d \n", client_sockfd);
                    pthread_create(&thread_a, NULL, thread_function, (void *)client_sockfd);
                }
            }
        }
    }
    return 0;
}
