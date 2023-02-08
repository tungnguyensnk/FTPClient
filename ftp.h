#ifndef FTPCLIENT_FTP_H
#define FTPCLIENT_FTP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "connect.h"

struct data {
    char *data;
    int size;
};

struct data* recv_data(int socket);

struct ip setPASV(int sfd);

char *get_current_dir(int sfd);

#endif //FTPCLIENT_FTP_H
