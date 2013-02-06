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
#include <poll.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_CLIENT 3


int main() {
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    char buffer[128];

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET; // TCP/IP
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // client IP constrant
    server_address.sin_port = htons(9734); //port
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

    listen(server_sockfd, 5);

    //no wait time to reuse the socket
    int bReuseaddr = 1;
    setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&bReuseaddr, sizeof(bReuseaddr));

    printf("server waiting\n");

    struct pollfd fds[MAX_CLIENT];
    fds[0].fd = server_sockfd;
    fds[0].events = POLLIN|POLLPRI;
    int fd_num = 1;

    while(1) {
        int poll_value = poll(fds, fd_num, 500);
        if( poll_value < 0) {
            printf("poll < 0\n");
            return (-1);
        } else if(poll_value == 0) { //nothing input
            //printf("poll == 0, fd_num= %d \n", fd_num);
        }
        if(fds[0].revents) { //new client connect
            client_len = sizeof(client_address);
            client_sockfd = accept(fds[0].fd, (struct sockaddr *)&client_address, &client_len);
            if(client_sockfd < 0)
            {
                perror("accept");
                return (-1);
            }
            else if (fd_num >= MAX_CLIENT)
            {
                printf("Too many users \n");
            }
            else
            { //accept correct
                printf("add client socketfd fd_num = %d\n", fd_num);
                // add client fd
                fds[fd_num].fd = client_sockfd;
                fds[fd_num].events = POLLIN|POLLPRI;
                fd_num++;
                //non-block to first recv
                fds[fd_num].revents = 0;
            }
        }
        // check data from client socketfd
        int i = 1;
        while (i < fd_num) {
            if(fds[i].revents) {
                int read_value = recv(fds[i].fd, &buffer, sizeof(buffer), 0);
                if ( read_value > 0)
                {
                    printf("client_sockfd recv %s\n", buffer);
                    buffer[0]++;
                    send(fds[i].fd, &buffer, sizeof(buffer), 0);
                    printf("client_sockfd send %s\n", buffer);
                }
                //client disconnect
                else if(read_value == 0)
                {
                    if(i!= (fd_num - 1)){
                        fds[i] = fds[fd_num];
                    }
                    fd_num--;
                }
                //recv error
                else{
                    perror("recv");
                    return (-1);
                }
                fds[i].revents = 0;
                bzero(buffer, sizeof(buffer));
            }
            i++;
        }
    }
    return 0;
}


