#include <ev.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void recv_string(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
    char buffer[256];
    int read = recv(watcher->fd, buffer, sizeof(buffer), 0);

    //read error
    if (read < 0)
    {
        perror("read ");
        return;
    }
    //client disconnect
    else if (read == 0)
    {
        ev_io_stop(loop, watcher);
        free(watcher);
        printf("close a connection\n");
        return;
    }
    printf("server recv %s \n",buffer);
    buffer[0]++;
    send(watcher->fd, buffer, read, 0);
}

void accept_client(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
    struct sockaddr_in client_address;
    int client_len = sizeof(client_address);
    int client_sockfd;
    struct ev_io *w_client = (struct ev_io*) malloc (sizeof(struct ev_io));

    client_sockfd = accept( watcher->fd, (struct sockaddr *)&client_address, &client_len);

    //accept client error
    if(client_sockfd < 0)
    {
        perror("accept ");
        return;
    }

    //add watcher to connect client fd
    ev_io_init(w_client, recv_string, client_sockfd, EV_READ);
    ev_io_start(loop, w_client);
    printf("accept client\n");
}

int main(void)
{
    int server_sockfd;
    int server_len;
    struct sockaddr_in server_address;
    struct ev_loop *loop = ev_default_loop(0);
    struct ev_io stdin_watcher;

    //create server socket & bind it to socket address
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET; // TCP/IP
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // client IP constrant
    server_address.sin_port = htons(9734); //port
    server_len = sizeof(server_address);

    //no wait time to reuse the socket
    int bReuseaddr = 1;
    setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&bReuseaddr, sizeof(bReuseaddr));

    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    //listen on server socket
    listen(server_sockfd, 5);

    //create watcher to accept connection
    ev_io_init(&stdin_watcher, accept_client, server_sockfd, EV_READ);
    ev_io_start(loop, &stdin_watcher);
    printf("server is ready\n");

    //start event loop
    while (1)
    {
        ev_loop(loop, 0);
    }

    return 0;
}
