#define _GNU_SOURCE
#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define PATH_SIZE           2048
#define BUFFER_SIZE         4096
#define HTTP_200_TEMPLATE   "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n"
#define HTTP_404_TEMPLATE   "HTTP/1.1 404 Not Found\r\n\r\n"
#define HTTP_500_TEMPLATE   "HTTP/1.1 500 Internal Server Error\r\n\r\n"

char *getmimetype(const char *filename) {
    const char *fileext = strrchr(filename, '.');
    if (fileext != NULL) {
        if (strcmp(fileext, ".txt") == 0) {
            return "text/plain";
        }
        if (strcmp(fileext, ".html") == 0 || strcmp(fileext, ".htm") == 0) {
            return "text/html";
        }
        if (strcmp(fileext, ".css") == 0) {
            return "text/css";
        }
        if (strcmp(fileext, ".js") == 0) {
            return "text/javascript";
        }
        if (strcmp(fileext, ".png") == 0) {
            return "image/png";
        }
        if (strcmp(fileext, ".jpg") == 0 || strcmp(fileext, ".jpeg") == 0) {
            return "image/jpg";
        }
    }
    return "application/octet-stream";
}

int getresource(char *req, char *res, int size) {
    char *saveptr1, *saveptr2;
    char *line = strtok_r(req, "\r\n", &saveptr1);
    if (line == NULL) {
        return -1;
    }
    char *method = strtok_r(line, " ", &saveptr2);
    char *path = strtok_r(NULL, " ", &saveptr2);
    if (method == NULL || path == NULL) {
        return -1;
    }
    char *querystr = strchr(path, '?');
    if (querystr != NULL) {
        *querystr = '\0';
    }
    if (strcmp(path, "/") == 0) {
        snprintf(res, size, "/index.html");
    } else {
        snprintf(res, size, "%s", path);
    }
    return 0;
}

int sendhttpres(int socket, int code, FILE *file, char *mimetype) {
    char buffer[BUFFER_SIZE] = {0};
    size_t bytesread;

    switch (code) {
        case 200:
            snprintf(buffer, BUFFER_SIZE, HTTP_200_TEMPLATE, mimetype);
            write(socket, buffer, strlen(buffer));
            while ((bytesread = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                write(socket, buffer, bytesread);
            }
            break;
        case 404:
            snprintf(buffer, BUFFER_SIZE, HTTP_404_TEMPLATE);
            write(socket, buffer, strlen(buffer));                
            break;
        default:
            snprintf(buffer, BUFFER_SIZE, HTTP_500_TEMPLATE);
            write(socket, buffer, strlen(buffer));                
    }
    return 0;
}

int openhttpconn(struct http_conn* conn, int port, char *htdocs) {    
    conn->htdocs = htdocs;
    if ((conn->socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    int opts = 1;
    setsockopt(conn->socketfd, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts));
    conn->addr.sin_family = AF_INET;
    conn->addr.sin_addr.s_addr = INADDR_ANY;
    conn->addr.sin_port = htons(port);
    if (bind(conn->socketfd, (struct sockaddr*) &conn->addr, sizeof(conn->addr)) < 0){
        return -1;
    }
    if (listen(conn->socketfd, 10) < 0) {
        return -1;
    }
    return 0;
}

int closehttpconn(struct http_conn* conn) {
    close(conn->socketfd);
    return 0;
}

void *listenhttpconn(void *args) {
    struct http_conn* conn = (struct http_conn*) args;
    char buffer[BUFFER_SIZE], resource[PATH_SIZE], filepath[PATH_SIZE];
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int socket = accept(conn->socketfd, (struct sockaddr*) &client_addr, &addrlen);
        if (socket < 0) {
            break; 
        }
        memset(buffer, '\0', sizeof(buffer));
        if (read(socket, buffer, BUFFER_SIZE - 1) < 0) {
            sendhttpres(socket, 500, NULL, NULL);
            close(socket);
            continue;
        }
        if (getresource(buffer, resource, PATH_SIZE) < 0) {
            sendhttpres(socket, 500, NULL, NULL);
            close(socket);
            continue;
        }
        if (strstr(resource, "..") != NULL) {
            sendhttpres(socket, 404, NULL, NULL);
            close(socket);
            continue;
        }
        printf("[INFO][worker-%ld] Request: %s\n", (long)pthread_self(), resource);
        snprintf(filepath, PATH_SIZE, "%s%s", conn->htdocs, resource);
        FILE *file = fopen(filepath, "rb");
        if (!file) {
            sendhttpres(socket, 404, NULL, NULL);
            close(socket);
            continue;
        }
        sendhttpres(socket, 200, file, getmimetype(resource));
        fclose(file);
        close(socket);
    }
    return NULL;
}
