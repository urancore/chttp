#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>

#include "inc/platform.h"
#include "inc/server.h"
#include "inc/parser.h"
#include "inc/response.h"
#include "inc/handler.h"

// TODO: add error types*
typedef struct  {
	chttp_socket_t sock;
	ChttpServer *server;
} ClientContext;

VOID CALLBACK ClientWorker(PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WORK work)
{
	(void)instance;

	ClientContext *ctx = (ClientContext*)context;
	ChttpServer *server = ctx->server;
	chttp_socket_t client = ctx->sock;

	char *read_buffer = malloc(server->read_buffer_size);
	char *write_buffer = malloc(server->write_buffer_size);

	if (!read_buffer || !write_buffer) {
		goto cleanup;
	}

	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char*)&server->read_timeout, sizeof(server->read_timeout));
	setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, (char*)&server->write_timeout, sizeof(server->write_timeout));

	int bytes_received = chttp_read(client, read_buffer, server->read_buffer_size - 1);
	if (bytes_received == SOCKET_ERROR) {
		int err = WSAGetLastError();
		if (err == WSAETIMEDOUT) {
			printf("[TIMEOUT] read took longer than %ld ms\n", server->read_timeout);
		} else {
			printf("[ERROR] recv failed: %d\n", err);
		}
		fflush(stdout);
		goto cleanup;
	}

	if (bytes_received == 0) {
		printf("[INFO] Connection closed by client\n");
		fflush(stdout);
		goto cleanup;
	}

	read_buffer[bytes_received] = '\0';

	printf("[OK] Received %d bytes\n", bytes_received);
	fflush(stdout);

	ChttpRequest request = {0};
	if (!chttp_request_parse(&request, read_buffer)) {
		goto cleanup;
	}

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
	CloseThreadpoolWork(work);
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

		ClientContext *ctx = (ClientContext*)malloc(sizeof(ClientContext));
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
