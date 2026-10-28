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
#define HTTP_404_TEMPLATE   "HTTP/1.1 404 Not Found\r\n"
#define HTTP_500_TEMPLATE   "HTTP/1.1 500 Internal Server Error\r\n"

char *getMimeType(const char *filename) {
    const char *fileExtension = strrchr(filename, '.');
    if (fileExtension != NULL) {
        if (strcmp(fileExtension, ".txt") == 0) {
            return "text/plain";
        }
        if (strcmp(fileExtension, ".html") == 0 || strcmp(fileExtension, ".htm") == 0) {
            return "text/html";
        }
        if (strcmp(fileExtension, ".css") == 0) {
            return "text/css";
        }
        if (strcmp(fileExtension, ".js") == 0) {
            return "text/javascript";
        }
        if (strcmp(fileExtension, ".xml") == 0) {
            return "application/xml";
        }
        if (strcmp(fileExtension, ".json") == 0) {
            return "application/json";
        }
        if (strcmp(fileExtension, ".png") == 0) {
            return "image/png";
        }
        if (strcmp(fileExtension, ".jpg") == 0 || strcmp(fileExtension, ".jpeg") == 0) {
            return "image/jpg";
        }
    }
    return "application/octet-stream";
}

int getUrlResource(char *request, char *resource, int maxSize) {
    char *saveptr1, *saveptr2;
    char *line = strtok_r(request, "\r\n", &saveptr1);
    if (line == NULL) {
        return -1;
    }
    char *method = strtok_r(line, " ", &saveptr2);
    if (method == NULL) {
        return -1;
    }
    char *path = strtok_r(NULL, " ", &saveptr2);
    if (path == NULL) {
        return -1;
    }
    char *queryString = strchr(path, '?');
    if (queryString != NULL) {
        *queryString = '\0';
    }
    if (strcmp(path, "/") == 0) {
        snprintf(resource, maxSize, "/index.html");
    } else {
        snprintf(resource, maxSize, "%s", path);
    }
    return 0;
}

int sendHttpResponse(int socketClient, int statusCode, FILE *file, char *mimeType) {
    char buffer[BUFFER_SIZE] = {0};
    size_t bytesRead;

    switch (statusCode) {
        case 200:
            snprintf(buffer, BUFFER_SIZE, HTTP_200_TEMPLATE, mimeType);
            write(socketClient, buffer, strlen(buffer));
            while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                write(socketClient, buffer, bytesRead);
            }
            break;
        case 404:
            snprintf(buffer, BUFFER_SIZE, HTTP_404_TEMPLATE);
            write(socketClient, buffer, strlen(buffer));                
            break;
        default:
            snprintf(buffer, BUFFER_SIZE, HTTP_500_TEMPLATE);
            write(socketClient, buffer, strlen(buffer));                
    }
    return 0;
}

int openHttpConnection(HttpConn *conn, int port, char *htdocs) {    
    int socketOptions;        
    conn->htdocs = htdocs;
    if ((conn->socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {        
        return -1;
    }
    socketOptions = 1;
    if (setsockopt(conn->socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &socketOptions, sizeof(socketOptions)) < 0) {        
        return -1;
    }
    conn->address.sin_family = AF_INET;
    conn->address.sin_addr.s_addr = INADDR_ANY;
    conn->address.sin_port = htons(port);
    if (bind(conn->socketDescriptor, (struct sockaddr*)&conn->address, sizeof(conn->address)) < 0) {
        return -1;
    }
    if (listen(conn->socketDescriptor, 1) < 0) {
        return -1;
    }
    return 0;
}

int closeHttpConnection(HttpConn *conn) {
    close(conn->socketDescriptor);
    return 0;
}

void *listenHttpRequest(void *args) {
    HttpConn *conn = (HttpConn*) args;
    char buffer[BUFFER_SIZE],
         resource[PATH_SIZE],
         filepath[PATH_SIZE];
    
    while (1) {
        socklen_t addressLength = sizeof(conn->address);
        int socketClient = accept(conn->socketDescriptor, (struct sockaddr*) &conn->address, &addressLength);
        if (socketClient < 0) {
            perror("[ERROR] Error accepting incoming request");
            continue;
        }
        memset(buffer, '\0', sizeof(buffer));
        if (read(socketClient, buffer, BUFFER_SIZE - 1) < 0) {
            perror("[ERROR] Error reading incoming request");
            sendHttpResponse(socketClient, 500, NULL, NULL);
            close(socketClient);
            continue;
        }
        if (getUrlResource(buffer, resource, PATH_SIZE) < 0) {
            perror("[ERROR] Error getting URL resource");
            sendHttpResponse(socketClient, 500, NULL, NULL);
            close(socketClient);
            continue;
        }
        printf("[INFO][worker-%ld] Receiving HTTP request: %s\n", (pthread_t) pthread_self(), resource);
        snprintf(filepath, PATH_SIZE, "%s%s", conn->htdocs, resource);
        FILE *file = fopen(filepath, "rb");
        if (!file) {
            fprintf(stderr, "[ERROR] Error reading file: %s\n", filepath);
            sendHttpResponse(socketClient, 404, NULL, NULL);
            close(socketClient);
            continue;
        }
        sendHttpResponse(socketClient, 200, file, getMimeType(resource));
        close(socketClient);
        fclose(file);
    }
    return NULL;
}
