#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "http.h"

// Basic config:
#define PORT                8080
#define THREAD_POOL_SIZE    10
#define HTDOCS              "./htdocs"

void stop();

pthread_t threadPool[THREAD_POOL_SIZE];
volatile int run = 1;

int main() {
    HttpConn *conn;
    int i;

    if (signal(SIGINT, stop) == SIG_ERR) {
        perror("[ERROR] Error setting up Ctrl+C signal handler");
        return -1;
    }    
    conn = (HttpConn*) malloc(sizeof(HttpConn));
    if (openHttpConnection(conn, PORT, HTDOCS) < 0) {        
        perror("[ERROR] Error connuring HTTP connection");
        free(conn);
        return -1;
    }
    printf("[INFO] HTTP server listening on port %d, press Ctrl+C to abort.\n", PORT);
    for (i = 0; i < THREAD_POOL_SIZE; i++) {
        if (pthread_create(&threadPool[i], NULL, listenHttpRequest, (void *) conn)) {
            perror("[ERROR] Error creating thread");
            free(conn);
            return -1;
        }
    }
    while (run) {
        sleep(1);
    }
    printf("\n[INFO] Shut down all working threads...\n");    
    for (i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_cancel(threadPool[i]);
        pthread_join(threadPool[i], NULL);
    }
    printf("[INFO] Closing HTTP connection...\n");
    closeHttpConnection(conn);
    free(conn);
    printf("[INFO] Done.\n");
    printf("[INFO] Bye!\n");
    return 0;
}

void stop() {
    run = 0;
}
