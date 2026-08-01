#include <string.h>

#include "inc/types.h"
#include "inc/helper.h"

#define CHAR_DIFF_MASK 0x20 // 32

// if true return 1 else 0
int equal_str(char *str1, char *str2)
{
	char c1, c2;
	while ((c1 = *str1++) && (c2 = *str2++)) {
		if ((c1 ^ c2) != 0 && (c1 ^ c2) != CHAR_DIFF_MASK)
			return 0;
	}

	return c1 == c2;
}

int equaln_str(char *str1, char *str2, size_t max_count)
{
	char c1, c2;
	size_t counter = 0;

	while (counter < max_count && (c1 = *str1++) && (c2 = *str2++)) {
		if ((c1 ^ c2) != 0 && (c1 ^ c2) != CHAR_DIFF_MASK)
			return 0;
		counter++;
	}

	return 1;

}

// if key exists return val index in headers else -1
int chttp_header_get_val(ChttpHeader *headers, int hcount,  char *key)
{
	for (int i = 0; i < hcount; i++) {
		if (equaln_str(headers[i].key, key, headers[i].key_len)) {
			return i;
		}
	}

	return -1;
}
