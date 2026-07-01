#include <string.h>
#include <stdio.h>

#include "inc/parser.h"

char *find_eol(char *s)
{
	if (s == NULL) return NULL;

	char *crlf = strstr(s, "\r\n");
	char *lf = strchr(s, '\n');

	if (!crlf) return lf;

	if (!lf) return crlf;

	return (crlf < lf) ? crlf : lf;
}

int eol_len(char *eol)
{
	if (eol == NULL || eol[0] == '\0' )
		return 0;

	if (eol[0] == '\r' && eol[1] == '\n')
		return 2;

	if (eol[0] == '\n')
		return 1;

	return 0;
}

// return len crlf/lf
int eol_header_end(char *eol)
{
	if (eol == NULL || eol[0] == '\0')
		return 0;

	if (eol[0] == '\r' && eol[1] == '\n') {
		if (eol[2] == '\r' && eol[3] == '\n')
			return 4;

		if (eol[2] == '\n')
			return 3;
	}

	if (eol[0] == '\n' && eol[1] == '\n')
		return 2;

	return 0;
}

int parse_req_line(char *line, int line_len,
                   char **method, int *method_len,
                   char **url, int *url_len,
                   char **http, int *http_len)
{
	if (!line || !method || !method_len || !url || !url_len || !http || !http_len)
		return -1;

	char *start = line;
	char *end_of_line = line + line_len;

	char *space1 = memchr(start, ' ', end_of_line - start);
	if (!space1 || space1 == start) {
		return -2;
	}

	*method = start;
	*method_len = space1 - start;

	char *url_start = space1 + 1;
	if (url_start >= end_of_line) {
		return -3;
	}

	char *space2 = memchr(url_start, ' ', end_of_line - url_start);
	if (!space2 || space2 == url_start) {
		return -4;
	}


	*url = url_start;
	*url_len = space2 - url_start;

	char *http_start = space2 + 1;
	if (http_start >= end_of_line) {
		return -5;
	}

	*http = http_start;
	*http_len = end_of_line - http_start;

	if (*http_len == 0) {
		return -6;
	}

	return 0;
}

ChttpMethod _get_method(char *method, int method_len)
{
	if (method == NULL) return CHTTP_UNKNOWN;

	if (method_len == 3 && !strncmp(method, "GET", 3)) return CHTTP_GET;
	if (method_len == 4 && !strncmp(method, "POST", 4)) return CHTTP_POST;

	return CHTTP_UNKNOWN;
}

ChttpVersion _get_http_ver(char *http)
{
	if (!strncmp(http, "HTTP/1.0", 8)) return HTTP_VER_1_0;
	if (!strncmp(http, "HTTP/1.1", 8)) return HTTP_VER_1_1;
	if (!strncmp(http, "HTTP/2.0", 8)) return HTTP_VER_2_0;
	if (!strncmp(http, "HTTP/3.0", 8)) return HTTP_VER_3_0;

	return HTTP_VER_UNKNOWN;
}

int chttp_parse_request(ChttpRequest *req, char *str)
{
	if (str == NULL) return 0;

	char *ptr = str;
	char *end = 0;

	char *method;
	int method_len;

	char *url;
	int url_len;

	char *http;
	int http_len;

	// parse request line
	end = find_eol(ptr);
	if (!end) return 0;

	if (parse_req_line(ptr, end - ptr, &method, &method_len, &url, &url_len, &http, &http_len) != 0)
		return 0;

	ChttpMethod chttp_method = _get_method(method, method_len);
	if (chttp_method == CHTTP_UNKNOWN)
		return 0;

	req->method = chttp_method;
	req->url = url;
	req->url_len = url_len;
	req->http_version = _get_http_ver(http);

	int hindex = 0;
	ptr = end + eol_len(end);
	int body_found = 0;

	// parse headers
	while(ptr && *ptr != '\0') {
		end = find_eol(ptr);
		if (!end) return 0;

		if (ptr == end || (end && ptr == end)) {
			int header_end = eol_header_end(ptr);
			if (header_end <= 0) {
				header_end = eol_len(ptr);
			}

			req->body = ptr + header_end;
			req->body_len = strlen(req->body);

			body_found = 1;
			break;
		}


		if (hindex < CHTTP_MAX_HEADERS) {
			char *colon = strchr(ptr, ':');
			if (colon && colon < end) {
				req->headers[hindex].key = ptr;
				req->headers[hindex].key_len = colon - ptr;

				char *val = colon + 1;
				while (val < end && (*val == ' ' || *val == '\t')) {
					val++;
				}

				req->headers[hindex].val = val;
				req->headers[hindex].val_len = end - val;

				hindex++;
			} else {
				return 0;
			}
		}

		ptr = end + eol_len(end);
	}

	if (!body_found)
		return 0;

	req->headers_count = hindex;
	return 1;
}
