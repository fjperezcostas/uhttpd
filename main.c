#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "http.h"

// Basic config:
#define PORT            8080
#define NUM_THREADS     100
#define HTDOCS          "./htdocs"

HttpConfig *config;
pthread_t threadPool[NUM_THREADS];    

void cancelAllThreads();

int main() {
    int i;

    if (signal(SIGINT, cancelAllThreads) == SIG_ERR) {
        perror("Error setting up Ctrl+C signal handler");
        return -1;
    }
    if ((config = configHttpConnection(PORT, HTDOCS)) == NULL) {
        perror("Error configuring HTTP connection");
        return -1;
    }
    printf("HTTP server listening on port %d, press Ctrl+C to abort.\n", PORT);
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threadPool[i], NULL, listenHttpRequest, (void *)config);
    }
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threadPool[i], NULL);
    }
    printf("Closing HTTP connection...\n");
    closeHttpConnection(config);
    printf("Done.\n");
    printf("Bye!\n");
    return 0;
}

void cancelAllThreads() {
    int i;

    printf("Shut down all working threads...\n");
    for (i = 0; i <  NUM_THREADS; i++) {
        pthread_cancel(threadPool[i]);
    }
    printf("Done.\n");
}
