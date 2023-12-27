#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "http.h"

// Basic config:
#define PORT            8080
#define NUM_THREADS     25              // Size of threadpool, feel free to configure to fit your hardware requierements
#define HTDOCS          "./htdocs"

void cancelAllThreads();

pthread_t threadPool[NUM_THREADS];

int main() {
    HttpConfig *config;
    int i;

    if (signal(SIGINT, cancelAllThreads) == SIG_ERR) {
        perror("[ERROR] Error setting up Ctrl+C signal handler");
        return -1;
    }
    if ((config = configHttpConnection(PORT, HTDOCS)) == NULL) {
        perror("[ERROR] Error configuring HTTP connection");
        return -1;
    }
    printf("[INFO] HTTP server listening on port %d, press Ctrl+C to abort.\n", PORT);
    for (i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threadPool[i], NULL, listenHttpRequest, (void *)config)) {
            perror("[ERROR] Error creating thread");
            return -1;
        }
    }
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threadPool[i], NULL);
    }
    printf("[INFO] Closing HTTP connection...\n");
    closeHttpConnection(config);
    printf("[INFO] Done.\n");
    printf("[INFO] Bye!\n");
    return 0;
}

void cancelAllThreads() {
    int i;

    printf("\n[INFO] Shut down all working threads...\n");
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_cancel(threadPool[i]);
    }
    printf("[INFO] Done.\n");
}
