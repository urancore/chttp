#include <string.h>
#include <stdio.h>
#include <dirent.h>

#include "chttp.h"

void my_log(const char *message) {
	printf("%s\n", message);
}

void mainpage_handler(ChttpResponse *resp, ChttpRequest *req) {
	(void)req;
	chttp_response_write_string(resp, "HTTP/1.1 200 OK\r\n");
	chttp_response_set_header(resp, "1232313", "12312312312312");
	chttp_response_set_header(resp, "Hello-World", "hihihi");
	chttp_response_set_header(resp, "ABOBA", "ABOBA2006");
	chttp_response_write_string(resp, "\r\n");
	chttp_response_write_string(resp, "<h1>Hello world</h1>"); // body
	chttp_response_flush(resp);
}

void upload_handler(ChttpResponse *resp, ChttpRequest *req) {
	(void)req;
	chttp_response_write_string(resp, "HTTP/1.1 200 OK\r\n");
	chttp_response_write_string(resp, "\r\n");
	chttp_response_write_string(resp, "<h1>UPLOADED</h1>"); // body
	chttp_response_flush(resp);
}

int main(void)
{
	ChttpServer server = {0};
	server.addr = "127.0.0.1";
	server.port = 5000;

	server.read_buffer_size = 0x100000; // 1MiB
	server.write_buffer_size = 0x10000; // 64KiB
	server.max_header_size = 0x10000; // 64KiB
	server.max_body_size = 0x10000; // 64KiB


	server.read_timeout = 10000; // 10 sec
	server.write_timeout = 10000; // 10 sec
	server.idle_timeout = 30000; // 30 sec

	server.max_connections = SOMAXCONN;

	server.logger = my_log;

	ChttpRouter router = {0};
	chttp_route(&router, CHTTP_GET, "/", mainpage_handler);
	chttp_route(&router, CHTTP_POST, "/upload", upload_handler);

	server.router = &router;

	if (!chttp_server_run(&server)) {
		printf("[ERROR] failed start server\n");
		return 1;
	}

	return 0;
}
