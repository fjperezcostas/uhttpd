#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>  // Añadido para las conexiones fantasma
#include <netinet/in.h>  // Añadido para las conexiones fantasma
#include <arpa/inet.h>   // Añadido para las conexiones fantasma
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
    printf("[INFO] Shutting down: waking up all threads from accept()...\n");    
    closehttpconn(&conn);
    // review this part
    for (i = 0; i < THREAD_POOL_SIZE; i++) {
        int dummy_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (dummy_socket >= 0) {
            struct sockaddr_in dummy_addr;
            dummy_addr.sin_family = AF_INET;
            dummy_addr.sin_port = htons(PORT);
            dummy_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            connect(dummy_socket, (struct sockaddr*)&dummy_addr, sizeof(dummy_addr));
            close(dummy_socket);
        }
    }
    printf("[INFO] Joining threads...\n");
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
