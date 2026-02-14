#ifndef HTTP_H
#define HTTP_H

typedef struct {
    char method[8];
    char path[128];
    char version[16];
    long content_length;
    char *body_start;
} HTTP_Request;

void parse_request(char *buffer, HTTP_Request *req);

#endif