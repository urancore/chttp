#ifndef CHTTP_IO_H
#define CHTTP_IO_H

#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>

typedef int socket_t;

static inline int chttp_write(int fd, const char* data, size_t len) {
    return (int)write(fd, data, len);
}

#endif
