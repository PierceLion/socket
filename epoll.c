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
#include <sys/epoll.h>

#define MAX_CLIENT 2

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
    //setnonblocking(server_sockfd);
    printf("server waiting\n");

    //create epoll fd
    int epollfds = epoll_create(MAX_CLIENT);
    if(epollfds < 0)
        perror("epoll create");

    //set server_socket into the epoll fd.
    struct epoll_event ev, events[3];
    int fds = 0, i=0;
    ev.events = EPOLLIN|EPOLLPRI|EPOLLET;
    ev.data.fd = server_sockfd;
    if(epoll_ctl(epollfds, EPOLL_CTL_ADD, server_sockfd, &ev) < 0){
        perror("epoll_ctl:");
    }
    //
    int client_num = 0;

    while(1){
        fds = epoll_wait(epollfds, events, 3, 500);
        if (fds == -1) { //epoll error
            perror("epoll_wait");
            return(-1);
        }
        i=0;
        while(i < fds){
            if(events[i].data.fd == server_sockfd) { //new client connect
                printf("It's a server fd recv event\n");
                if(client_num >= MAX_CLIENT) {
                    client_len = sizeof(client_address);
                    int client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
                    send(client_sockfd, "Too many users", 14, 0);
                    close(client_sockfd);
                }
                else{
                    client_len = sizeof(client_address);
                    int client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
                    printf("accept client fd=%d \n", client_sockfd);
                    if(client_sockfd < 0) {
                        perror("accept ");
                        return(-1);
                    }
                    client_num++;
                    ev.events = EPOLLIN|EPOLLPRI;
                    ev.data.fd = client_sockfd;
                    epoll_ctl(epollfds, EPOLL_CTL_ADD, client_sockfd, &ev);
                }
            } else {
                int read_num = recv(events[i].data.fd, &buffer, sizeof(buffer), 0);
                if(read_num < 0) { //read error
                    perror("read ");
                    return (-1);
                } else if(read_num == 0) { //client disconnect
                    ev.events = EPOLLIN|EPOLLPRI;
                    ev.data.fd = client_sockfd;
                    epoll_ctl(epollfds, EPOLL_CTL_DEL, events[i].data.fd, &ev);
                    printf("delete client fd %d \n", events[i].data.fd);
                    close(events[i].data.fd);
                    client_num--;
                } else { //access correct
                    printf("client_sockfd recv %s\n", buffer);
                    buffer[0]++;
                    send(events[i].data.fd, &buffer, sizeof(buffer), 0);
                    printf("client_sockfd send %s\n", buffer);
                }
                bzero(buffer, sizeof(buffer));
            }
            i++;
        }
    }
    return 0;
}
