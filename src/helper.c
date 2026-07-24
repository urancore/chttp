#include <string.h>

#include "inc/types.h"

// if key exists return val index in headers else -1
int chttp_header_get_val(ChttpHeader *headers, int hcount,  char *key)
{
	for (int i = 0; i < hcount; i++) {
		if (!strncmp(headers[i].key, key, headers[i].key_len)) {
			return i;
		}
	}

	return -1;
}
