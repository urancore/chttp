#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>

#include "inc/platform.h"
#include "inc/server.h"
#include "inc/parser.h"
#include "inc/response.h"
#include "inc/handler.h"
#include "inc/helper.h"

typedef struct  {
	chttp_socket_t sock;
	ChttpServer *server;
} _ClientContext;

#define ERROR_RESP(client, str) chttp_write(client, str, strlen(str))

static void server_log(ChttpServer *server, const char *fmt, ...) {
	if (!server || !server->logger) return;

	char buffer[512];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	server->logger(buffer);
}

static int read_headers(chttp_socket_t client, ChttpServer *server,
				char *buffer, int buffer_size, int *out_header_size)
{
	int total_bytes = 0;
	int bytes_received = 0;
	char *headers_end = NULL;

	while (total_bytes < buffer_size - 1) {
		int space_left = buffer_size - total_bytes - 1;

		bytes_received = chttp_read(client, buffer + total_bytes, space_left);
		if (bytes_received == SOCKET_ERROR) {
			int err = WSAGetLastError();
			if (err == WSAETIMEDOUT) {
				server_log(server, "error: timeout while reading headers");
			} else {
				server_log(server, "error: read headers failed: %d", err);
			}
			return -1;
		} else if (bytes_received == 0) {
			server_log(server, "error: connection closed while reading headers");
			return -1;
		}

		total_bytes += bytes_received;
		buffer[total_bytes] = '\0';

		headers_end = strstr(buffer, "\r\n\r\n");
		if (headers_end != NULL) {
			*out_header_size = (int)(headers_end - buffer) + 4;
			return total_bytes;
		}

		headers_end = strstr(buffer, "\n\n");
		if (headers_end != NULL) {
			*out_header_size = (int)(headers_end - buffer) + 2;
			return total_bytes;
		}

		if (total_bytes > server->max_header_size) {
			server_log(server, "error: headers exceed max size %d", server->max_header_size);
			ERROR_RESP(client, "HTTP/1.1 431 Request Header Fields Too Large\r\nConnection: close\r\n\r\n");
			return -2;
		}
	}

	server_log(server, "error: headers buffer overflow");
	ERROR_RESP(client, "HTTP/1.1 431 Request Header Fields Too Large\r\nConnection: close\r\n\r\n");
	return -2;
}

static int read_body(chttp_socket_t client, ChttpServer *server,
			char *buffer, int buffer_size, int header_end_pos,
			size_t content_length, size_t *out_body_received)
{
	int body_start_pos = header_end_pos;
	size_t body_already_received = buffer_size - header_end_pos;

	if (body_already_received > content_length) {
		body_already_received = content_length;
	}

	*out_body_received = body_already_received;

	if (body_already_received >= content_length) {
		return 0;
	}

	size_t bytes_to_read = content_length - body_already_received;

	while (bytes_to_read > 0) {
		size_t chunk_size = (bytes_to_read > 4096) ? 4096 : bytes_to_read;

		if (body_start_pos + *out_body_received + chunk_size > (size_t)buffer_size) {
			server_log(server, "error: body exceeds buffer size (have %zu, need %zu more)",
			          *out_body_received, chunk_size);
			chttp_write(client, "HTTP/1.1 413 Content Too Large\r\nConnection: close\r\n\r\n",
			           strlen("HTTP/1.1 413 Content Too Large\r\nConnection: close\r\n\r\n"));
			return -1;
		}

		int bytes_received = chttp_read(client, buffer + body_start_pos + *out_body_received,
		                                (int)chunk_size);
		if (bytes_received == SOCKET_ERROR) {
			int err = WSAGetLastError();
			if (err == WSAETIMEDOUT) {
				server_log(server, "error: timeout while reading body");
			} else {
				server_log(server, "error: read body failed: %d", err);
			}
			return -1;
		} else if (bytes_received == 0) {
			server_log(server, "error: connection closed while reading body (got %zu/%zu bytes)",
			          *out_body_received, content_length);
			return -1;
		}

		*out_body_received += bytes_received;
		bytes_to_read -= bytes_received;

		server_log(server, "info: body chunk received: %d bytes (total %zu/%zu)",
		          bytes_received, *out_body_received, content_length);
	}

	return 0;
}

static int parse_content_length(ChttpRequest *request, ChttpServer *server,
					size_t *out_content_length)
{
	*out_content_length = 0;

	int vali = chttp_header_get_val(request->headers, request->headers_count, "Content-Length");
	if (vali < 0) {
		return 0; // not found
	}

	char tmp_str[32];
	if (request->headers[vali].val_len >= (int)sizeof(tmp_str)) {
		server_log(server, "error: Content-Length value too long");
		return -1;
	}

	memcpy(tmp_str, request->headers[vali].val, request->headers[vali].val_len);
	tmp_str[request->headers[vali].val_len] = '\0';

	char *endptr = NULL;
	size_t content_length = strtoull(tmp_str, &endptr, 10);

	if (endptr == tmp_str || *endptr != '\0') {
		server_log(server, "error: invalid Content-Length: %s", tmp_str);
		return -1;
	}

	if (content_length > server->max_body_size) {
		server_log(server, "error: Content-Length %zu exceeds max_body_size %zu",
		          content_length, server->max_body_size);
		return -2;
	}

	*out_content_length = content_length;
	return 0;
}


VOID CALLBACK ClientWorker(PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WORK work)
{
	(void)work;
	DisassociateCurrentThreadFromCallback(instance);

	_ClientContext *ctx = (_ClientContext*)context;
	ChttpServer *server = ctx->server;
	chttp_socket_t client = ctx->sock;

	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char*)&server->read_timeout, sizeof(server->read_timeout));
	setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, (char*)&server->write_timeout, sizeof(server->write_timeout));

	char *read_buffer = malloc(server->read_buffer_size);
	char *write_buffer = malloc(server->write_buffer_size);

	if (!read_buffer || !write_buffer) {
		server_log(server, "error: memory allocation failed for client socket %d", client);
		goto cleanup;
	}

	int header_size = 0;
	int total_bytes_read = read_headers(client, server, read_buffer,
	                                     server->read_buffer_size, &header_size);

	if (total_bytes_read == -2) {
		goto cleanup;
	} else if (total_bytes_read < 0) {
		goto cleanup;
	}

	server_log(server, "info: headers read, size: %d bytes, total received: %d bytes",
	          header_size, total_bytes_read);

	ChttpRequest request = {0};
	if (!chttp_request_parse(&request, read_buffer)) {
		server_log(server, "error: request parse failed");
		ERROR_RESP(client, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n");
		goto cleanup;
	}

	size_t content_length = 0;
	int err_code = parse_content_length(&request, server, &content_length);

	if (err_code == -2) {
		ERROR_RESP(client, "HTTP/1.1 413 Content Too Large\r\nConnection: close\r\n\r\n");
		goto cleanup;
	} else if (err_code < 0) {
		ERROR_RESP(client, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n");
		goto cleanup;
	}

	size_t body_received = 0;

	if (content_length > 0) {
		err_code = read_body(client, server, read_buffer, server->read_buffer_size,
		                     header_size, content_length, &body_received);

		if (err_code == -1) {
			goto cleanup;
		}
	}

	request.body = read_buffer + header_size;
	request.body_len = body_received;

	server_log(server, "DEBUG: method: %d, url: %.*s",
	          request.method, request.url_len, request.url);
	server_log(server, "DEBUG: body offset: %d bytes", header_size);
	server_log(server, "DEBUG: body length: %zu bytes", request.body_len);
	server_log(server, "DEBUG: total received: %d bytes", total_bytes_read);

	ChttpResponse response = {
		.sock = client,
		.server = server,
		.write_buffer = write_buffer,
		.write_pos = 0
	};

	chttp_dispatch(server->router, &response, &request);

cleanup:
	if (read_buffer) free(read_buffer);
	if (write_buffer) free(write_buffer);

	closesocket(client);
	free(ctx);
}

int chttp_server_run(ChttpServer *server)
{
	WSADATA wsa_data;

	chttp_socket_t main_sock = INVALID_SOCKET;
	SOCKADDR_IN server_addr;

	int err = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (err != NO_ERROR) {
		printf("[ERROR] WSAStartup: failed with error: %d\n", err);
		return 0;
	}

	main_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (main_sock == INVALID_SOCKET) {
		printf("[ERROR] failed init socket\n");
		return 0;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(server->addr);
	server_addr.sin_port = htons(server->port);

	// set opt to reuse ports
	int reuse = 1;
	setsockopt(main_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

	err = bind(main_sock, (SOCKADDR *)&server_addr, sizeof(server_addr));
	if (err == SOCKET_ERROR) {
		printf("[ERROR] bind: failed with error %d\n", WSAGetLastError());
		err = closesocket(main_sock);
		if (err == SOCKET_ERROR)
			printf("[ERROR] closesocket: failed with error %d\n", WSAGetLastError());

		WSACleanup();
		return 0;
	}

	if (listen(main_sock, server->max_connections) == SOCKET_ERROR) {
		printf("[ERROR] listen: failed with error: %d\n", WSAGetLastError());
		closesocket(main_sock);
		WSACleanup();

		return 0;
	}

	for (;;) {
		chttp_socket_t client_sock = accept(main_sock, NULL, 0);
		if (client_sock == INVALID_SOCKET) {
			continue;
		}

		_ClientContext *ctx = (_ClientContext*)malloc(sizeof(_ClientContext));
		if (!ctx) {
			closesocket(client_sock);
			continue;
		}

		ctx->sock = client_sock;
		ctx->server = server;

		PTP_WORK work = CreateThreadpoolWork(ClientWorker, ctx, NULL);
		if (work) {
			SubmitThreadpoolWork(work);
		} else {
			free(ctx);
			closesocket(client_sock);
		}

	}

	closesocket(main_sock);
	WSACleanup();

	return 1;
}
