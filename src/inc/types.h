#ifndef CHTTP_TYPES_H
#define CHTTP_TYPES_H

#define CHTTP_MAX_HEADERS 64
#define MAX_ROUTES 64

#if defined(_WIN32) || defined(_WIN64)
	#if defined(_WIN64)
		typedef unsigned __int64 chttp_socket_t;
	#else
		typedef unsigned int chttp_socket_t;
	#endif
#else
	typedef int chttp_socket_t;
#endif

typedef enum {
	CHTTP_GET,
	CHTTP_POST,
	CHTTP_PUT,
	CHTTP_PATCH,
	CHTTP_DELETE,
	CHTTP_UNKNOWN
} ChttpMethod;

typedef enum {
	HTTP_VER_UNKNOWN = 0,
	HTTP_VER_1_0,
	HTTP_VER_1_1,
	HTTP_VER_2_0,
	HTTP_VER_3_0
} ChttpVersion;

typedef struct KeyVal {
	char *key;
	int key_len;
	char *val;
	int val_len;
} KeyVal;

typedef KeyVal ChttpHeader;

typedef struct
{
	ChttpMethod method;
	char *url; // TODO: *add typedef struct URL
	int url_len;
	ChttpVersion http_version;

	ChttpHeader headers[CHTTP_MAX_HEADERS];
	int headers_count;

	char *body;
	int body_len;
} ChttpRequest;

typedef struct ChttpServer ChttpServer;
typedef struct ChttpResponse ChttpResponse;
typedef struct ChttpRouter ChttpRouter;
typedef struct ChttpRoute ChttpRoute;

typedef void (*ChttpLoggerFunc)(const char *message);

typedef struct ChttpServer {
	int port;
	char *addr;

	int read_buffer_size;
	int write_buffer_size;
	int max_header_size;
	size_t max_body_size;

	unsigned long read_timeout;
	unsigned long write_timeout;
	int idle_timeout;

	int max_connections;

	ChttpRouter *router;

	ChttpLoggerFunc logger;
} ChttpServer;

typedef struct ChttpResponse {
	chttp_socket_t sock;
	ChttpServer *server;

	char *write_buffer;
	unsigned long long write_pos;
} ChttpResponse;

typedef void (*ChttpHandlerFunc)(ChttpResponse *resp, ChttpRequest *req);

typedef struct ChttpRoute {
	ChttpMethod method;
	char *url;
	int url_len;
	ChttpHandlerFunc handler;
} ChttpRoute;

typedef struct ChttpRouter {
	ChttpRoute routes[MAX_ROUTES];
	int route_count;
} ChttpRouter;

#endif /* CHTTP_TYPES_H */
