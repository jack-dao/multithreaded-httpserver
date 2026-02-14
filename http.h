#ifndef HTTP_H
#define HTTP_H

typedef struct {
    char method[8];  // e.g., "GET" or "POST"
    char path[128];  // e.g., "/index.html"
    char version[16]; // e.g., "HTTP/1.1"
} HTTP_Request;

void parse_request(char *raw_request, HTTP_Request *req);

#endif