#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include "tcp.h"

#define abs(x) ((x) < 0 ? -(x) : (x))

static volatile int stop = 0;

void usage(void)
{
    fprintf(stderr, "tcping version 0.3\n");

    fprintf(stderr, "Usage: tcping [-fqhv] [-p(4|6)] [-c count] [-i interval] [-I interface] destination port\n");

    fprintf(stderr, "-c count   How many times to connect\n");
    fprintf(stderr, "-t timeout	timeout for the connection\n");
    fprintf(stderr, "-i interval	delay between each connect\n");
    fprintf(stderr, "-I interface	interface to connect via\n");
    fprintf(stderr, "-f		flood connect (no delays)\n");
    fprintf(stderr, "-q		quiet, only returncode\n");
    fprintf(stderr, "-p(4|6)        prever ipv4/ipv6\n");
    fprintf(stderr, "-h     This help text\n\n");
}

void handler(int sig)
{
    fprintf(stderr, "\nreceive signal:[%d]\n", sig);
    stop = 1;
}

int main(int argc, char *argv[])
{
    char *hostname = NULL;
    char *portnr = (char *)"80";
    char * interface;
    int c;
    int count = -1, curncount = 0;
    int wait = 1, quiet = 0, timeout = 60;
    int ok = 0, err = 0;
    double min = 999999999999999.0, avg = 0.0, max = 0.0;
    struct addrinfo *resolved;
    int errcode;
    int seen_addrnotavail;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_protocol = 0;

    while((c = getopt(argc, argv, "hp:c:i:t:I:fq?")) != -1)
    {
        switch(c)
        {

            case 'c':
                count = atoi(optarg);
                break;

            case 'i':
                wait = atoi(optarg);
                break;

            case 't' :
                timeout = atoi(optarg);
                break;
            case 'I' :
                interface = optarg;
                break;

            case 'f':
                wait = 0;
                break;

            case 'q':
                quiet = 1;
                break;

            case 'p':
                 if(strcmp(optarg, "4") ){
                      hints.ai_family = AF_INET6;
                }
                else if(strcmp(optarg, "6")){
                     hints.ai_family = AF_INET;
                }
                break;
            case '?':
            default:
                usage();
                return 0;
        }
    }

    if (optind >= argc)
    {
        fprintf(stderr, "ERROR: No hostname given\n");
        usage();
        return 3;
    }

    hostname = argv[optind];
    portnr = argv[optind+1];

    signal(SIGINT, handler);
    signal(SIGTERM, handler);

    if ((errcode = getaddrinfo(hostname, portnr, &hints, &resolved)) != 0)
    {
        fprintf(stderr, "%s\n", gai_strerror(errcode));
        return 2;
    }
    char address[2+INET6_ADDRSTRLEN];
    if( resolved->ai_addr->sa_family == AF_INET6 ) {
        address[0]='[';
        struct sockaddr_in6 *addr;
        addr = (struct sockaddr_in6 *)resolved->ai_addr; 
        inet_ntop(AF_INET6, &(addr->sin6_addr), address+1, INET6_ADDRSTRLEN);
        address[strlen(address)]=']';
    }else{
        struct sockaddr_in *addr;
        addr = (struct sockaddr_in *)resolved->ai_addr; 
        inet_ntop(AF_INET, &(addr->sin_addr), address, INET_ADDRSTRLEN);
    }

    if (!quiet)
        printf("TCPING %s (%s):%s\n", hostname, address, portnr);

    while((curncount < count || count == -1) && stop == 0)
    {
        double ms;
        struct timeval rtt;

        if ((errcode = connect_to(resolved, interface,  timeout, &rtt)) != 0)
        {
            err++;
            if (errcode != -EADDRNOTAVAIL)
            {
                printf("error connecting to host (%d): %s", -errcode, strerror(-errcode));
                if(-errcode == EINPROGRESS){
                    printf(" [Timeout]\n");
                }else{
                    printf("\n");
                }
            }
            else
            {
                if (seen_addrnotavail)
                {
                    printf(".");
                    fflush(stdout);
                }
                else
                {
                    printf("error connecting to host (%d): %s\n", -errcode, strerror(-errcode));
                }
                seen_addrnotavail = 1;
            }
        }
        else
        {
            seen_addrnotavail = 0;
            ok++;

            ms = ((double)rtt.tv_sec * 1000.0) + ((double)rtt.tv_usec / 1000.0);
            avg += ms;
            min = min > ms ? ms : min;
            max = max < ms ? ms : max;

            printf("response from %s:%s, seq=%d time=%.2f ms\n", address, portnr, curncount, ms);
            if (ms > 5000) break; /* Stop the test on the first long connect() */
        }

        curncount++;

        if (curncount != count)
            sleep(wait);
    }

    if (!quiet)
    {
        printf("--- %s:%s tcping statistics ---\n", hostname, portnr);
        printf("%d responses, %d ok, %3.2f%% failed\n", curncount, ok, (((double)err) / abs(((double)curncount)) * 100.0));
        printf("round-trip min/avg/max = %.1f/%.1f/%.1f ms\n", min, avg / (double)ok, max);
    }

    freeaddrinfo(resolved);
    if (ok)
        return 0;
    else
        return 127;
}
