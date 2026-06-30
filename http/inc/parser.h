#ifndef CHTTP_PARSER_H
#define CHTTP_PARSER_H 1

#include <stdlib.h>

#define MAX_HEADERS 64

typedef enum {
	CHTTP_GET,
	CHTTP_POST,
	CHTTP_UNKNOWN
} ChttpMethod;

typedef enum {
    HTTP_VER_UNKNOWN = 0,
    HTTP_VER_1_0,
    HTTP_VER_1_1,
    HTTP_VER_2_0,
    HTTP_VER_3_0
} ChttpVersion;

typedef struct {
	char *key;
	size_t key_len;
	char *val;
	size_t val_len;
} ChttpHeader;

typedef struct
{
	ChttpMethod method;
	char *url; // TODO: *add typedef struct URL
	size_t url_len;
	ChttpVersion http_version;

	ChttpHeader headers[MAX_HEADERS];
	size_t headers_count;

	char *body;
	size_t body_len;
} ChttpRequest;

int parse_request(ChttpRequest *req, char *str);

#endif
