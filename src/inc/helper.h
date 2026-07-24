#ifndef CHTTP_HELPER_H
#define CHTTP_HELPER_H

#include "types.h"

int chttp_header_get_val(ChttpHeader *headers, int hcount, char *key);

#endif
