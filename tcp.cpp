#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "tcp.h"

int lookup(char *host, char *portnr, struct addrinfo **res)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_protocol = 0;

    return getaddrinfo(host, portnr, &hints, res);
}

int connect_to(struct addrinfo *addr, char * interface, int timeout, struct timeval *rtt)
{
    struct timeval start, tv;

    const int on = 1;
    int rv = 0;
    fd_set fdset;

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    for (addrinfo *iter = addr; iter != NULL; iter = iter->ai_next ){
        
        Descriptor fd(socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol));
        if (fd == -1)
            continue;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
            continue;
        setsockopt( fd, SOL_SOCKET, SO_BINDTODEVICE, interface, sizeof(interface));
        fcntl(fd, F_SETFL, O_NONBLOCK);

        if (gettimeofday(&start, NULL) == -1)
            continue;

        /* connect to peer */
        connect(fd, addr->ai_addr, addr->ai_addrlen);

        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);

        if (select(fd + 1, NULL, &fdset, NULL, &tv) == 1)
        {
            int so_error;
            socklen_t len = sizeof so_error;

            getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &len);

            if (gettimeofday(rtt, NULL) == -1)
                continue;

            rtt->tv_sec = rtt->tv_sec - start.tv_sec;
            rtt->tv_usec = rtt->tv_usec - start.tv_usec;

            if (so_error == 0) {
                return 0;
            } else {
                rv = -ETIMEDOUT;
            }
        }
    }

    rv = rv ? rv : -errno;
    return rv;
}
