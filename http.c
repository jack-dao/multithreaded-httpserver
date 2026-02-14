#include <stdio.h>
#include <string.h>
#include "http.h"

void parse_request(char *raw_request, HTTP_Request *req) {
    sscanf(raw_request, "%s %s %s", req->method, req->path, req->version);
}
