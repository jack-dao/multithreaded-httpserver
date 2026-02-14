#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h> // For open()
#include "http.h"

#define PORT 8080
#define BUFFER_SIZE 1024

const char *get_mime_type(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot) return "text/plain";
    
    if (strcmp(dot, ".html") == 0) return "text/html";
    if (strcmp(dot, ".css") == 0)  return "text/css";
    if (strcmp(dot, ".js") == 0)   return "application/javascript";
    if (strcmp(dot, ".jpg") == 0)  return "image/jpeg";
    if (strcmp(dot, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(dot, ".png") == 0)  return "image/png";
    if (strcmp(dot, ".gif") == 0)  return "image/gif";
    
    return "text/plain";
}

void send_file(int client_socket, const char *filename) {
    FILE *file = fopen(filename, "rb");
    
    if (file == NULL) {
        char *not_found = "HTTP/1.1 404 Not Found\r\n\r\n404: File Not Found";
        write(client_socket, not_found, strlen(not_found));
        return;
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    const char *mime_type = get_mime_type(filename);

    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header), 
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "\r\n", mime_type, fsize);

    write(client_socket, header, strlen(header));

    char file_buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0) {
        write(client_socket, file_buffer, bytes_read);
    }

    fclose(file);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int opt = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 10);
    
    printf("Server listening on port %d...\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        char buffer[BUFFER_SIZE] = {0};
        read(new_socket, buffer, BUFFER_SIZE);
        
        HTTP_Request req;
        parse_request(buffer, &req);
        
        if (strcmp(req.path, "/") == 0) {
            send_file(new_socket, "index.html");
        } else {
            send_file(new_socket, req.path + 1);
        }

        close(new_socket);
    }
    close(server_fd);
    return 0;
}