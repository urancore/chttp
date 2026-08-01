#ifndef CHTTP_HELPER_H
#define CHTTP_HELPER_H

#include "types.h"

int chttp_header_get_val(ChttpHeader *headers, int hcount, char *key);
int equal_str(char *str1, char *str2);
int equaln_str(char *str1, char *str2, size_t max_count);

#endif
