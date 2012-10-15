Rationale
---

    man select
    
    BUGS


       Under  Linux, select() may report a socket file descriptor as "ready for reading", while nevertheless a subsequent
       read blocks.  This could for example happen when data has arrived but upon examination has wrong checksum  and  is
       discarded.   There may be other circumstances in which a file descriptor is spuriously reported as ready.  Thus it
       may be safer to use O_NONBLOCK on sockets that should not block.

Effectively it means "select can fire just because of it wants so", so all applications should use nonblocking sockets and be ready to ignore spurious selections.

Howerer I see blocking file descriptors being placed to `select` here and there.

This is library that makes `select` and `poll` additionally return ready status for file descriptors by random chance, simplifying finding out applications using blocking sockets and select/poll simultaneously.

Usage
---

    make
    LD_PRELOAD=`pwd`/libselectfuzz.so my_program