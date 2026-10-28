#ifndef HTTP_H
#define HTTP_H

#include <arpa/inet.h>

struct http_conn {
    int                 socketfd;
    struct sockaddr_in  addr;
    char*               htdocs;
};

int openhttpconn(struct http_conn*, int, char*);
int closehttpconn(struct http_conn*);
void *listenhttpconn(void*);

#endif
