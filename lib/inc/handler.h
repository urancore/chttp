#ifndef CHTTP_HANDLER_H
#define CHTTP_HANDLER_H

#include "types.h"
#include "platform.h"

#define MAX_ROUTES 64

typedef void (*HandlerFunc)(ChttpRequest *req, socket_t sock);

typedef struct {
	ChttpMethod method;
	char *url;
	int url_len;
	HandlerFunc handler;
} Route;

typedef struct {
	Route routes[MAX_ROUTES];
	int route_count;
} Router;

void route(Router *router, ChttpMethod method, char *url, int ulr_size, HandlerFunc handler);
void dispatch(Router *router, ChttpRequest *req, socket_t socket);

#endif
