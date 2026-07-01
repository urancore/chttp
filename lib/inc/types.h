#ifndef CHTTP_TYPES_H
#define CHTTP_TYPES_H

#define CHTTP_MAX_HEADERS 64

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
	int key_len;
	char *val;
	int val_len;
} ChttpHeader;

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

#endif
