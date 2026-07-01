#ifndef CHTTP_IO_H
#define CHTTP_IO_H

#include <winsock2.h>

typedef int socket_t;

static inline int chttp_write(socket_t sock, const char* data, size_t len) {
	return send(sock, data, (int)len, 0);
}

#endif
