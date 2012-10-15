#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <sys/time.h>
#include <sys/select.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

static int (*orig_select)(int nfds, fd_set *readfds, fd_set *writefds,
                  fd_set *exceptfds, struct timeval *timeout) = NULL;
static int (*orig_pselect) (int nfds, fd_set *readfds, fd_set *writefds,
                   fd_set *exceptfds, const struct timespec *timeout,
                   const sigset_t *sigmask) = NULL;

int rnd = -1;
           
           
unsigned char random_byte() {
    if (rnd == -1) {
       rnd = open("/dev/urandom", O_RDONLY);
    }
    unsigned char b;
    read(rnd, &b, 1);
    return b;
}
                   
int fuzzfds(int nfds, fd_set* fds) {
    if(!fds) return 0;
    int i;
    int counter = 0;
    for(i=0; i<nfds; ++i) {
        if(FD_ISSET(i, fds) && random_byte() < 100) {
            // retain set
            ++counter;
        } else {
            FD_CLR(i, fds);
        }
    }
    return counter;
}
                   
int select(int nfds, fd_set *readfds, fd_set *writefds,
                  fd_set *exceptfds, struct timeval *timeout) {
    if(!orig_select) { 
        orig_select = dlsym(RTLD_NEXT, "select");
    }

    if(random_byte() < 80) {
        if(exceptfds) FD_ZERO(exceptfds);
        int counter = fuzzfds(nfds, readfds) + fuzzfds(nfds, writefds);
        return counter;
    }
    
    int ret = orig_select(nfds, readfds, writefds, exceptfds, timeout);
    
    return ret;
}
int pselect(int nfds, fd_set *readfds, fd_set *writefds,
        fd_set *exceptfds, const struct timespec *timeout,
        const sigset_t *sigmask) {
    if(!orig_pselect) {
        orig_pselect = dlsym(RTLD_NEXT, "pselect");
    }

    
    if(random_byte() < 80) {
        if(exceptfds) FD_ZERO(exceptfds);
        int counter = fuzzfds(nfds, readfds) + fuzzfds(nfds, writefds);
        return counter;
    }
    
    int ret = orig_pselect(nfds, readfds, writefds, exceptfds, timeout, sigmask);
    return ret;
}

