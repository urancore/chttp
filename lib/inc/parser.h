#ifndef CHTTP_PARSER_H
#define CHTTP_PARSER_H

#include "types.h"

int chttp_parse_request(ChttpRequest *req, char *str);

#endif
