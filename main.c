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

pthread_t threadpool[THREAD_POOL_SIZE];
volatile int run = 1;

int main() {
    struct http_conn conn;
    int i;

    if (signal(SIGINT, stop) == SIG_ERR) {
        perror("[ERROR] Error setting up Ctrl+C signal handler");
        return -1;
    }    
    if (openhttpconn(&conn, PORT, HTDOCS) < 0) {        
        perror("[ERROR] Error configuring HTTP connection");
        return -1;
    }
    printf("[INFO] HTTP server listening on port %d, press Ctrl+C to abort.\n", PORT);
    for (i = 0; i < THREAD_POOL_SIZE; i++) {
        if (pthread_create(&threadpool[i], NULL, listenhttpconn, &conn)) {
            perror("[ERROR] Error creating thread");
            return -1;
        }
    }
    while (run) {
        sleep(1);
    }
    printf("[INFO] Closing connection...\n");    
    closehttpconn(&conn);
    printf("[INFO] Cancelling all threads...\n");
    for (i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_cancel(threadpool[i]);
    }
    printf("[INFO] Joining all threads...\n");
    for (i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_join(threadpool[i], NULL);
    }
    printf("[INFO] Done.\n");
    printf("[INFO] Bye!\n");
    return 0;
}

void stop() {
    run = 0;
}
