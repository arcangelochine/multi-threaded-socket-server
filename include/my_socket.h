#ifndef MY_SOCKET_H
#define MY_SOCKET_H

#include <my_utils.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>     
#include <arpa/inet.h>
#include <netinet/in.h>   

typedef struct sockaddr_in Address;

/**
 * @brief Creates a socket
 * 
 * @param socket_fd socket's fd pointer
*/
void Socket(int *socket_fd)
{
    if ((errno = 0, *socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        Perror("socket");
}

void ReuseAddr(int socket_fd)
{
    int reuse_addr = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) == -1)
        Perror("setsockopt");
}

/**
 * @brief Sets up a AF_INET address to localhost:port
 * 
 * @param addr address pointer
 * @param port port of the address
*/
void SetAddr(Address *addr, const char *addr_name, uint16_t port)
{
    memset(addr, 0, sizeof(Address));
    addr->sin_family = AF_INET;
    if (addr_name == NULL)
        addr->sin_addr.s_addr = htonl(INADDR_ANY);
    else
        addr->sin_addr.s_addr = inet_addr(addr_name);
    addr->sin_port = htons(port);
}

/**
 * @brief Binds a socket's fd to an address
 * 
 * @param socket_fd socket's fd to bind
 * @param addr address pointer to bind
*/
void Bind(int socket_fd, Address *addr)
{
    errno = 0;
    if (bind(socket_fd, (struct sockaddr *) addr, sizeof(*addr)) == -1)
        Perror("bind");
}

/**
 * @brief Makes the socket to listen n requests
 * 
 * @param socket_fd socket's fd to listen on
 * @param n number of request before refusing connections
*/
void Listen(int socket_fd, int n)
{
    errno = 0;
    if (listen(socket_fd, n) == -1)
        Perror("listen");
}

/**
 * @brief Makes a server to accept new connections
 * @note Infos about client's address are lost, use Accept_a instead
 * 
 * @param client_fd socket's fd pointer for the communication
 * @param server_fd server's fd
*/
void Accept(int *client_fd, int server_fd)
{
    if ((errno = 0, *client_fd = accept(server_fd, NULL, NULL)) == -1)
        Perror("accept");
}

void Accept_a(int *client_fd, int server_fd, Address *client_addr, socklen_t *addr_len)
{
    if ((errno = 0, *client_fd = accept(server_fd, (struct sockaddr *)client_addr, addr_len)) == -1)
        Perror("accept");
}

/**
 * @brief Attempts to connect a client_fd socket to a passive socket
 * @note Tries an attempt every 1 sec if keeps getting ECONNREFUSED error
*/
void Connect(int client_fd, Address *server_addr)
{
    errno = 0;
    int status = 0;
    
    while ((status = connect(client_fd, (struct sockaddr *) server_addr, sizeof(Address))) == -1 && errno == ECONNREFUSED)
        sleep(1);

    if (status == -1)
        Perror("connect");
}

void Send(int socket_fd, const void *buffer, size_t length)
{
    errno = 0;
    if (send(socket_fd, buffer, length, 0) == -1)
        Perror("send");
}

void Receive(int socket_fd, void *buffer, size_t length)
{
    errno = 0;
    if (recv(socket_fd, buffer, length, 0) == -1)
        Perror("recv");
}

#endif
