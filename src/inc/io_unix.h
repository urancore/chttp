#ifndef CHTTP_IO_H
#define CHTTP_IO_H

#include <stdlib.h>

#include "types.h"

static inline int chttp_write(chttp_socket_t fd, const char *data, size_t len) {
	return (int)write(fd, data, len);
}

#endif
