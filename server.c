#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "http.h" 

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd;
    int new_socket;
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
        printf("\nWaiting for a connection...\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE] = {0};
        read(new_socket, buffer, BUFFER_SIZE);
        
        HTTP_Request req;
        parse_request(buffer, &req);
        
        printf("Method: %s\n", req.method);
        printf("Path: %s\n", req.path);
        printf("Version: %s\n", req.version);
        // -------------------------

        char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
        write(new_socket, hello, strlen(hello));
        close(new_socket);
    }
    close(server_fd);
    return 0;
}
