#ifndef FTPCLIENT_CONNECT_H
#define FTPCLIENT_CONNECT_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>

struct ip {
    char ip[1024];
    int port;
};

int connect_tcp(struct ip ip);

struct ip create_ip(char *ip_str, int port_num);

#endif //FTPCLIENT_CONNECT_H