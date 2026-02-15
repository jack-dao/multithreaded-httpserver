#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <pthread.h>
#include "http.h"
#include "queue.h"

#define PORT 8080
#define BUFFER_SIZE 4096
#define THREAD_POOL_SIZE 20
#define NUM_LOCKS 127

queue_t client_queue;
pthread_rwlock_t file_locks[NUM_LOCKS];

unsigned long hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; 
    return hash % NUM_LOCKS;
}

const char *get_mime_type(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot) return "text/plain";
    if (strcmp(dot, ".html") == 0) return "text/html";
    if (strcmp(dot, ".css") == 0)  return "text/css";
    if (strcmp(dot, ".js") == 0)   return "application/javascript";
    if (strcmp(dot, ".jpg") == 0)  return "image/jpeg";
    if (strcmp(dot, ".png") == 0)  return "image/png";
    return "text/plain";
}

void handle_get(int client_socket, char *filename) {
    unsigned long lock_index = hash(filename);

    pthread_rwlock_rdlock(&file_locks[lock_index]);
    
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        pthread_rwlock_unlock(&file_locks[lock_index]);
        char *not_found = "HTTP/1.1 404 Not Found\r\n\r\n404: File Not Found";
        write(client_socket, not_found, strlen(not_found));
        return;
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header), 
             "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", 
             get_mime_type(filename), fsize);
    write(client_socket, header, strlen(header));

    char file_buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0) {
        write(client_socket, file_buffer, bytes_read);
    }
    
    fclose(file);
    
    pthread_rwlock_unlock(&file_locks[lock_index]);
}

void handle_put(int client_socket, char *filename, long content_length, char *body_start) {
    unsigned long lock_index = hash(filename);

    pthread_rwlock_wrlock(&file_locks[lock_index]);

    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        pthread_rwlock_unlock(&file_locks[lock_index]);
        char *err = "HTTP/1.1 500 Internal Server Error\r\n\r\nError creating file";
        write(client_socket, err, strlen(err));
        return;
    }

    if (body_start != NULL && content_length > 0) {
        fwrite(body_start, 1, content_length, file); 
    }

    fclose(file);

    pthread_rwlock_unlock(&file_locks[lock_index]);

    char *response = "HTTP/1.1 201 Created\r\nContent-Length: 0\r\n\r\n";
    write(client_socket, response, strlen(response));
    
    printf("SUCCESS: Created file '%s' (Lock %lu)\n", filename, lock_index);
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    int bytes_read = read(client_socket, buffer, BUFFER_SIZE);
    
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }

    HTTP_Request req;
    parse_request(buffer, &req);
    
    char *filename = req.path + 1;
    if (strlen(filename) == 0) filename = "index.html";

    printf("DEBUG: Parsed Method='%s' Path='%s' Filename='%s'\n", req.method, req.path, filename);

    if (strcmp(req.method, "GET") == 0) {
        handle_get(client_socket, filename);
    } 
    else if (strcmp(req.method, "PUT") == 0) {
        handle_put(client_socket, filename, req.content_length, req.body_start);
    }
    else {
        char *resp = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
        write(client_socket, resp, strlen(resp));
    }

    close(client_socket);
}

void *thread_function(void *arg) {
    (void)arg;
    while (1) {
        int *pclient = dequeue(&client_queue);
        if (pclient != NULL) {
            handle_client(*pclient);
            free(pclient);
        }
    }
    return NULL;
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int opt = 1;

    init_queue(&client_queue);

    for (int i = 0; i < NUM_LOCKS; i++) {
        pthread_rwlock_init(&file_locks[i], NULL);
    }

    pthread_t thread_pool[THREAD_POOL_SIZE];
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&thread_pool[i], NULL, thread_function, NULL);
    }
    printf("Server running with %d sharded locks and %d threads.\n", NUM_LOCKS, THREAD_POOL_SIZE);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 10);
    
    while (1) {
        int *new_socket = malloc(sizeof(int));
        if ((*new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            free(new_socket);
            continue;
        }
        enqueue(&client_queue, new_socket);
    }
    
    close(server_fd);
    return 0;
}
