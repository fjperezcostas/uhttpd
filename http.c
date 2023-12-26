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

void *listenHttpRequest(void *arg) {
    HttpConfig *config;
    const char* delimiter = " ";
    char requestBuffer[BUFFER_SIZE] = {0}, 
         responseBuffer[BUFFER_SIZE] = {0},
         filename[FILENAME_LENGTH] = {0},
         *token,
         *saveptr;
    int socketClient;
    size_t bytesRead;
    socklen_t addressLength;

    config = (HttpConfig*)arg;
    addressLength = sizeof(config->address);
    while (1) {
        if ((socketClient = accept(config->socketDescriptor, (struct sockaddr*)&config->address, &addressLength)) < 0) {
            perror("Error accepting incomming request");
            return NULL;
        }
        if (read(socketClient, requestBuffer, BUFFER_SIZE) < 0) {
            close(socketClient);
            perror("Error reading incomming request");
            return NULL;
        }        
        token = strtok_r(requestBuffer, "\r\n", &saveptr);
        printf("[thread-%ld] Receiving HTTP request: %s\n", (pthread_t)pthread_self(), token);
        token = strtok_r(token, delimiter, &saveptr);
        token = strtok_r(NULL, delimiter, &saveptr);
        (strcmp(token, "/") == 0) ? sprintf(filename, "%s%s", config->htdocs, "/index.html") : sprintf(filename, "%s%s", config->htdocs, token);
        FILE *file = fopen(filename, "rb");
        if (!file) {
            sprintf(responseBuffer, HTTP_404_TEMPLATE);
            write(socketClient, responseBuffer, strlen(responseBuffer));
            close(socketClient);
            printf("%s\n", filename);
            perror("Error reading file");
            return NULL;
        }
        sprintf(responseBuffer, HTTP_200_TEMPLATE, filename);
        write(socketClient, responseBuffer, strlen(responseBuffer));
        while ((bytesRead = fread(responseBuffer, 1, sizeof(responseBuffer), file)) > 0) {
            write(socketClient, responseBuffer, bytesRead);
        }
        fclose(file);
        close(socketClient);
    }    
    return NULL;
}

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
