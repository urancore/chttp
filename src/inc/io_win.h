#ifndef CHTTP_IO_H
#define CHTTP_IO_H

#include "types.h"

static inline int chttp_write(chttp_socket_t sock, const char *data, size_t len) {
	return send(sock, data, (int)len, 0);
}

static inline int chttp_read(chttp_socket_t sock, char *buf, size_t len) {
	return recv(sock, buf, (int)len, 0);
}

#endif
