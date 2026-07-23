#ifndef CHTTP_RESPONSE_H
#define CHTTP_RESPONSE_H

#include "types.h"

void chttp_response_write_fmt(ChttpResponse *resp, char *fmt, ...);
void chttp_response_write_string(ChttpResponse *resp, char *str);
void chttp_response_set_header(ChttpResponse *resp, char *key, char *value);
void chttp_response_flush(ChttpResponse *resp);

#endif /* CHTTP_RESPONSE_H */
