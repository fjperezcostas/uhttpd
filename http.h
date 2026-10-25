#ifndef HTTP_H
#define HTTP_H

#include <arpa/inet.h>

typedef struct {
    int                 socketDescriptor;
    struct sockaddr_in  address;
    char                *htdocs;
} HttpConn;

int openHttpConnection(HttpConn*, int, char*);
int closeHttpConnection(HttpConn*);
void *listenHttpRequest(void*);

#endif
