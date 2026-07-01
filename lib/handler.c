#include <stdlib.h>
#include <string.h>

#include "inc/handler.h"

void route(Router *router, ChttpMethod method, char *url, int ulr_size, HandlerFunc handler)
{
	if (router->route_count >= MAX_ROUTES) return;

	router->routes[router->route_count].method = method;
	router->routes[router->route_count].url = url;
	router->routes[router->route_count].url_len = ulr_size;
	router->routes[router->route_count].handler = handler;
	router->route_count++;
}

void dispatch(Router *router, ChttpRequest *req, socket_t socket)
{
	for (int i = 0; i < router->route_count; i++) {
		if (router->routes[i].method == req->method
			&& strncmp(req->url, router->routes[i].url, router->routes[i].url_len) == 0) {
				router->routes[i].handler(req, socket);
				break;
		}
	}
}
