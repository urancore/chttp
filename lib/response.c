#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "inc/response.h"
#include "inc/platform.h"

void chttp_response_write_fmt(ChttpResponse *resp, char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	size_t remaining = resp->server->write_buffer_size - resp->write_pos;

	int written = vsnprintf(
		resp->write_buffer + resp->write_pos,
		remaining,
		fmt,
		args
	);

	va_end(args);

	if (written > 0) {
		resp->write_pos += written;
	}
}

void chttp_response_write_string(ChttpResponse *resp, char *str)
{
	chttp_response_write_fmt(resp, "%s", str);
}

void chttp_response_set_header(ChttpResponse *resp, char *key, char *value)
{
	chttp_response_write_fmt(resp, "%s: %s\r\n", key, value);
}

void chttp_response_flush(ChttpResponse *resp)
{
	if (resp->write_pos > 0) {
		chttp_write(resp->sock, resp->write_buffer, resp->write_pos);
		resp->write_pos = 0;
	}
}
