#include <stddef.h>

#include "types.h"

static void server_log(ChttpServer *server, const char *fmt, ...) {
	if (!server || !server->logger) return;

	char buffer[512];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	server->logger(buffer);
}
