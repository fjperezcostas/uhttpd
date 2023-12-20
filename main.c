#include <signal.h>
#include <stdio.h>
#include "http.h"

// basic config
#define PORT    8080
#define HTDOCS  "./htdocs"

volatile int quit = 0;

void shutdownServer();

int main() {
    HttpConfig *config;

    if (signal(SIGINT, shutdownServer) == SIG_ERR) {
        perror("Error setting up Ctrl+C signal handler");
        return -1;
    }
    if ((config = configHttpConnection(PORT, HTDOCS)) == NULL) {
        perror("Error configuring HTTP connection");
        return -1;
    }
    printf("HTTP server listening on port %d, press Ctrl+C to abort.\n", PORT);
    while (!quit) {
        if (listenHttpRequest(config) < 0) {
            perror("Error handling incomming HTTP request");
        }
    }
    printf("Shutdown HTTP server...\n");
    closeHttpConnection(config);
    printf("Done.\n");

    return 0;
}

void shutdownServer() {
    quit = 1;
}
