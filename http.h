#ifndef HTTP_H
#define HTTP_H

#include <arpa/inet.h>

typedef struct {
    int                 socketDescriptor;
    struct sockaddr_in  address;
    char                *htdocs;
} HttpConfig;

HttpConfig *configHttpConnection(int, char*);
int closeHttpConnection(HttpConfig*);
void *listenHttpRequest(void*);

#endif
