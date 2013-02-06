libev: libev_test.o
	gcc -o libev_test libev_test.o -L/usr/local/lib -Wall -rpath -Wl -lev 
client: client.o
	gcc -o client client.o
server: server.o
	gcc -o server server.o -lpthread
poll: poll.o
	gcc -o poll poll.o
epoll: epoll.o
	gcc -o epoll epoll.o
clean: 
	rm -rvf libev_test libev_test.o client client.o server server.o poll poll.o epoll epoll.o
