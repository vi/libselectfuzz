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
#include <poll.h>

#define FUZZ_PROB 80
// of 256
#define FDFUZZ_PROB 100

static int (*orig_select)(int nfds, fd_set *readfds, fd_set *writefds,
                  fd_set *exceptfds, struct timeval *timeout) = NULL;
static int (*orig_pselect) (int nfds, fd_set *readfds, fd_set *writefds,
                   fd_set *exceptfds, const struct timespec *timeout,
                   const sigset_t *sigmask) = NULL;
                   
static int (*orig_poll) (struct pollfd *fds, nfds_t nfds, int timeout) = NULL;
static int (*orig_ppoll) (struct pollfd *fds, nfds_t nfds,
               const struct timespec *timeout_ts, const sigset_t *sigmask);

                   
static int rnd = -1;
           
           
static unsigned char random_byte() {
    if (rnd == -1) {
       rnd = open("/dev/urandom", O_RDONLY);
    }
    unsigned char b;
    read(rnd, &b, 1);
    return b;
}
                   
static int fuzzfds(int nfds, fd_set* fds) {
    if(!fds) return 0;
    int i;
    int counter = 0;
    for(i=0; i<nfds; ++i) {
        if(FD_ISSET(i, fds) && random_byte() < FDFUZZ_PROB) {
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

    if(random_byte() < FUZZ_PROB) {
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

    
    if(random_byte() < FUZZ_PROB) {
        if(exceptfds) FD_ZERO(exceptfds);
        int counter = fuzzfds(nfds, readfds) + fuzzfds(nfds, writefds);
        return counter;
    }
    
    int ret = orig_pselect(nfds, readfds, writefds, exceptfds, timeout, sigmask);
    return ret;
}

static int fuzzpollfds(struct pollfd *fds, nfds_t nfds) {
    int i;
    int counter;
    for (i=0; i<nfds; ++i) {
        fds[i].revents = 0;
        int flag = 0;
        if ( (fds[i].events & POLLIN) && random_byte() < FDFUZZ_PROB) {
            fds[i].revents |= POLLIN;
            flag = 1;
        }
        if ( (fds[i].events & POLLOUT) && random_byte() < FDFUZZ_PROB) {
            fds[i].revents |= POLLOUT;
            flag = 1;
        }
        counter += flag;
    }
    return counter;
}



int poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    if(!orig_poll) {
        orig_poll = dlsym(RTLD_NEXT, "poll");
    }
    
    if(random_byte() < FUZZ_PROB) {
        return fuzzpollfds(fds, nfds);
    }
    
    return orig_poll(fds, nfds, timeout);
}

int ppoll(struct pollfd *fds, nfds_t nfds,
               const struct timespec *timeout_ts, const sigset_t *sigmask) {
    if(!orig_ppoll) {
        orig_ppoll = dlsym(RTLD_NEXT, "ppoll");
    }              
    
    if(random_byte() < FUZZ_PROB) {
        return fuzzpollfds(fds, nfds);
    }
    
    return orig_ppoll(fds, nfds, timeout_ts, sigmask);
}
