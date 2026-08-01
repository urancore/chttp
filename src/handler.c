#include <stdlib.h>
#include <string.h>

#include "inc/handler.h"
#include "inc/types.h"

void chttp_route(ChttpRouter *router, ChttpMethod method, char *url, ChttpHandlerFunc handler)
{
	if (router->route_count >= MAX_ROUTES) return;

	router->routes[router->route_count].method = method;
	router->routes[router->route_count].url = url;
	router->routes[router->route_count].url_len = strlen(url);
	router->routes[router->route_count].handler = handler;
	router->route_count++;
}

void chttp_dispatch(ChttpRouter *router, ChttpResponse *resp, ChttpRequest *req)
{
	for (int i = 0; i < router->route_count; i++) {
		if (router->routes[i].method == req->method
			&& strncmp(req->url.path, router->routes[i].url, router->routes[i].url_len) == 0) {
			router->routes[i].handler(resp, req);
			break;
		}
	}
}
