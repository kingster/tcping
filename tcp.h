#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>

#ifndef TCP_H_DEFINED
#define TCP_H_DEFINED

struct Descriptor
{
    Descriptor(int fd): fd_(fd)
    {
        if (fd < 0)
        {
            std::ostringstream oss;
            oss << "failed to open socket" << ": " << strerror(errno);
        }
    }
    ~Descriptor()
    {
        if (fd_ != -1 && close(fd_) == -1)
        {
            // throwing from destructors risks termination - avoid...
            std::cerr << "failed to close socket" << ": " << strerror(errno) << std::endl;
        }
    }
    operator int() const { return fd_; }

  private:
    int fd_;
};


int lookup(char *host, char *portnr, struct addrinfo **res);
int connect_to(struct addrinfo *addr, char * interface, int timeout, struct timeval *rtt);
#endif /* TCP_H_DEFINED */
