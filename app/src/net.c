#include "net.h"

#include <stdio.h>

#include "log.h"

#ifdef __WINDOWS__
  typedef int socklen_t;
#else
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <unistd.h>
# define SOCKET_ERROR -1
  typedef struct sockaddr_in SOCKADDR_IN;
  typedef struct sockaddr SOCKADDR;
  typedef struct in_addr IN_ADDR;
#endif

socket_t net_connect(Uint32 addr, Uint16 port) {
    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        perror("socket");
        return INVALID_SOCKET;
    }

    SOCKADDR_IN sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(addr);
    sin.sin_port = htons(port);

    if (connect(sock, (SOCKADDR *) &sin, sizeof(sin)) == SOCKET_ERROR) {
        perror("connect");
        return INVALID_SOCKET;
    }

    return sock;
}

socket_t net_listen(Uint32 addr, Uint16 port, int backlog) {
    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        perror("socket");
        return INVALID_SOCKET;
    }

    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *) &reuse, sizeof(reuse)) == -1) {
        perror("setsockopt(SO_REUSEADDR)");
    }

    SOCKADDR_IN sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(addr); // htonl() harmless on INADDR_ANY
    sin.sin_port = htons(port);

    if (bind(sock, (SOCKADDR *) &sin, sizeof(sin)) == SOCKET_ERROR) {
        perror("bind");
        return INVALID_SOCKET;
    }

    if (listen(sock, backlog) == SOCKET_ERROR) {
        perror("listen");
        return INVALID_SOCKET;
    }

    return sock;
}

socket_t net_accept(socket_t server_socket) {
    SOCKADDR_IN csin;
    socklen_t sinsize = sizeof(csin);
    return accept(server_socket, (SOCKADDR *) &csin, &sinsize);
}

ssize_t net_recv(socket_t socket, void *buf, size_t len) {
    return recv(socket, buf, len, 0);
}

ssize_t net_recv_all(socket_t socket, void *buf, size_t len) {
    return recv(socket, buf, len, MSG_WAITALL);
}

ssize_t net_send(socket_t socket, void *buf, size_t len) {
    return send(socket, buf, len, 0);
}

ssize_t net_send_all(socket_t socket, void *buf, size_t len) {
    ssize_t w;
    while (len > 0) {
        w = send(socket, buf, len, 0);
        if (w == -1) {
            return -1;
        }
        len -= w;
        buf += w;
    }
    return w;
}

SDL_bool net_shutdown(socket_t socket, int how) {
    return !shutdown(socket, how);
}
