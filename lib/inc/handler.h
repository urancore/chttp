#ifndef CHTTP_HANDLER_H
#define CHTTP_HANDLER_H

#include "types.h"

void chttp_route(ChttpRouter *router, ChttpMethod method, char *url, HandlerFunc handler);
void chttp_dispatch(ChttpRouter *router, ChttpResponse *resp, ChttpRequest *req);

#endif
