#include <stdio.h>
#include <winsock2.h>

#include "http/inc/parser.h"

void print_request(const ChttpRequest *req)
{
	printf("========== HTTP REQUEST INFO ==========\n");

	printf("Method:       ");
	switch (req->method) {
		case CHTTP_GET:  printf("GET\n"); break;
		case CHTTP_POST: printf("POST\n"); break;
		default:         printf("UNKNOWN\n"); break;
	}

	printf("URL:          %.*s (length: %zu)\n", (int)req->url_len, req->url, req->url_len);

	printf("HTTP Version: ");
	switch (req->http_version) {
		case HTTP_VER_1_0: printf("HTTP/1.0\n"); break;
		case HTTP_VER_1_1: printf("HTTP/1.1\n"); break;
		case HTTP_VER_2_0: printf("HTTP/2\n"); break;
		case HTTP_VER_3_0: printf("HTTP/3\n"); break;
		default:           printf("UNKNOWN\n"); break;
	}

	printf("Headers count: %zu\n", req->headers_count);
	for (size_t i = 0; i < req->headers_count; i++) {
		printf("  [%zu] %.*s: %.*s\n",
			i,
			(int)req->headers[i].key_len, req->headers[i].key,
			(int)req->headers[i].val_len, req->headers[i].val);
	}

	printf("Body length:  %zu\n", req->body_len);
	if (req->body_len > 0 && req->body != NULL) {
		printf("Body:\n%s\n", req->body);
	} else {
		printf("Body:         [EMPTY]\n");
	}
	printf("=======================================\n");
}

void handle_conn(SOCKET sock) {
	char buf[4096];
	int len = recv(sock, buf, sizeof(buf), 0);
	if (len > 0) {
		ChttpRequest req = {0};

		if (!parse_request(&req, buf)) {
			printf("[ERROR] invalid request\n");
			return;
		}

		print_request(&req);
	}
}

int main(void)
{
	WSADATA wsaData;
	int err = 0;

	err = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (err != NO_ERROR) {
		printf("[ERROR] WSAStartup failed: %d\n", err);
		return 1;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		printf("[ERROR] socket function failed with error: %d\n", WSAGetLastError());
	}

	struct sockaddr_in s_sockaddr = {0};
	s_sockaddr.sin_family = AF_INET;
	s_sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	s_sockaddr.sin_port = htons(5000);

	err = bind(sock, (SOCKADDR*)&s_sockaddr, sizeof(s_sockaddr));
	if (err == SOCKET_ERROR) {
		printf("[ERROR] bind failed: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	err = listen(sock, SOMAXCONN);
	if (err == SOCKET_ERROR) {
		printf("[ERROR] listening start failed: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	while (1) {
		struct sockaddr client_addr = {0};
		int addr_len = sizeof(client_addr);

		SOCKET client_sock = accept(sock, &client_addr, &addr_len);

		wchar_t ip_string[46] = { 0 }; // ip6 size

		if (client_sock == INVALID_SOCKET) {
			wprintf(L"[ERROR] accept failed: %d\n", WSAGetLastError());
			closesocket(sock);
			WSACleanup();
			return 1;
		} else {
			DWORD ip_buffer_len = 46;

			// convert the binary address structure to string
			if (WSAAddressToStringW((struct sockaddr*)&client_addr, sizeof(client_addr), NULL, ip_string, &ip_buffer_len) == 0) {
				printf("[INFO] new connection: %s, %ls\n", client_addr.sa_family == AF_INET ? "IP4" : "IP6", ip_string);
			} else {
				printf("[WARNING] Could not parse IP address. Error: %d\n", WSAGetLastError());
			}
		}

		handle_conn(client_sock);
		closesocket(client_sock);
	}

	closesocket(sock);
	WSACleanup();
}
