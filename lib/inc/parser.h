#ifndef CHTTP_PARSER_H
#define CHTTP_PARSER_H

#include "types.h"

int chttp_request_parse(ChttpRequest *req, char *str);

#endif
