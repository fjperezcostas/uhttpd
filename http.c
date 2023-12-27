#define _GNU_SOURCE
#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE         4096
#define FILENAME_LENGTH     2048
#define HTTP_200_TEMPLATE   "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n"
#define HTTP_404_TEMPLATE   "HTTP/1.1 404 Not Found\r\n"
#define HTTP_500_TEMPLATE   "HTTP/1.1 500 Internal Server Error\r\n"

char *getMimeType(const char *filename) {
    const char delimiter[] = ".";
    char filenameCopy[FILENAME_LENGTH] = {0}, 
         fileExtension[FILENAME_LENGTH] = {0},
         *token;

    strcpy(filenameCopy, filename);
    token = strtok(filenameCopy, delimiter);    
    while (token != NULL) {
        strcpy(fileExtension, token);     
        token = strtok(NULL, delimiter);
    }
    if (strcmp(fileExtension, "txt") == 0) {
        return "text/plain";
    }
    if (strcmp(fileExtension, "html") == 0 || strcmp(fileExtension, "htm") == 0) {
        return "text/html";
    }
    if (strcmp(fileExtension, "css") == 0) {
        return "text/css";
    }
    if (strcmp(fileExtension, "js") == 0) {
        return "text/javascript";
    }
    if (strcmp(fileExtension, "xml") == 0) {
        return "application/xml";
    }
    if (strcmp(fileExtension, "json") == 0) {
        return "application/json";
    }
    if (strcmp(fileExtension, "png") == 0) {
        return "image/png";
    }
    if (strcmp(fileExtension, "jpg") == 0 || strcmp(fileExtension, "jpeg") == 0) {
        return "image/jpg";
    }
    return "application/octet-stream";
}

char *getUrlResource(char *buffer, char *htdocs) {
    const char *delimiter = " ";
    char *filename, *token, *saveptr;

    token = strtok_r(buffer, "\r\n", &saveptr);
    if (token == NULL) {
        return NULL;
    }
    printf("[INFO][worker-%ld] Receiving HTTP request: %s\n", (pthread_t)pthread_self(), token);
    token = strtok_r(token, delimiter, &saveptr);
    if (token == NULL) {
        return NULL;
    }
    token = strtok_r(NULL, delimiter, &saveptr);
    if (token == NULL) {
        return NULL;
    }
    filename = (char*) malloc(sizeof(char) * FILENAME_LENGTH + 1);
    (strcmp(token, "/") == 0) ? sprintf(filename, "%s%s", htdocs, "/index.html") : sprintf(filename, "%s%s", htdocs, token);
    return filename;
}

int sendHttpResponse(int socketClient, int statusCode, FILE *file, char* mimeType) {
    char buffer[BUFFER_SIZE];
    size_t bytesRead;

    switch (statusCode) {
        case 200:
            sprintf(buffer, HTTP_200_TEMPLATE, mimeType);
            write(socketClient, buffer, strlen(buffer));
            while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                write(socketClient, buffer, bytesRead);
            }
            fclose(file);
            break;
        case 404:
            sprintf(buffer, HTTP_404_TEMPLATE);
            write(socketClient, buffer, strlen(buffer));                
            break;
        default:
            sprintf(buffer, HTTP_500_TEMPLATE);
            write(socketClient, buffer, strlen(buffer));                
    }
    close(socketClient);
    return 0;
}

HttpConfig *configHttpConnection(int port, char *htdocs) {
    HttpConfig *config;
    int socketOptions;
    
    config = (HttpConfig*)malloc(sizeof(HttpConfig));
    config->htdocs = htdocs;
    if ((config->socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        free(config);
        return NULL;
    }
    socketOptions = 1;
    if (setsockopt(config->socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &socketOptions, sizeof(socketOptions)) < 0) {
        free(config);
        return NULL;
    }
    config->address.sin_family = AF_INET;
    config->address.sin_addr.s_addr = INADDR_ANY;
    config->address.sin_port = htons(port);
    if (bind(config->socketDescriptor, (struct sockaddr*)&config->address, sizeof(config->address)) < 0) {
        free(config);
        return NULL;
    }
    if (listen(config->socketDescriptor, 1) < 0) {
        free(config);
        return NULL;
    }
    return config;
}

int closeHttpConnection(HttpConfig *config) {
    close(config->socketDescriptor);
    free(config);
    return 0;
}

void *listenHttpRequest(void *args) {
    HttpConfig *config;  
    int socketClient;
    socklen_t addressLength;    
    char buffer[BUFFER_SIZE], *filename;
    FILE *file;
    
    config = (HttpConfig*)args;
    addressLength = sizeof(config->address);
    while (1) {
        memset(buffer, '\0', sizeof(buffer));
        free(filename);
        if ((socketClient = accept(config->socketDescriptor, (struct sockaddr*)&config->address, &addressLength)) < 0) {
            perror("[ERROR] Error accepting incomming request");
            sendHttpResponse(socketClient, 500, NULL, NULL);
            continue;
        }
        if (read(socketClient, buffer, BUFFER_SIZE) < 0) {
            perror("[ERROR] Error reading incomming request");
            sendHttpResponse(socketClient, 500, NULL, NULL);
            continue;
        }
        filename = getUrlResource(buffer, config->htdocs);
        if (filename == NULL) {
            perror("[ERROR] Error getting URL resource");
            sendHttpResponse(socketClient, 500, NULL, NULL);
            continue;
        }
        file = fopen(filename, "rb");
        if (!file) {
            perror("[ERROR] Error reading file");
            sendHttpResponse(socketClient, 404, NULL, NULL);
            continue;
        }
        sendHttpResponse(socketClient, 200, file, getMimeType(filename));
    }
    return NULL;
}
