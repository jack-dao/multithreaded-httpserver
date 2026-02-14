#include "http.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void parse_request(char *buffer, HTTP_Request *req) {
    req->content_length = 0;
    req->body_start = NULL;

    sscanf(buffer, "%s %s %s", req->method, req->path, req->version);

    char *content_len_str = strstr(buffer, "Content-Length:");
    if (content_len_str) {
        sscanf(content_len_str, "Content-Length: %ld", &req->content_length);
    }

    char *body_separator = strstr(buffer, "\r\n\r\n");
    if (body_separator) {
        req->body_start = body_separator + 4;
    }
}