#include "ftp.h"

struct data *recv_data(int socket) {
    struct data *data = malloc(sizeof(struct data));
    data->data = malloc(1);
    data->size = 0;
    fd_set read_fds;
    struct timeval timeout;
    int bytes_received;

    FD_ZERO(&read_fds);
    FD_SET(socket, &read_fds);

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    char buffer[1024] = {0};

    int result = select(socket + 1, &read_fds, NULL, NULL, &timeout);
    if (result == -1) {
        perror("select");
        return NULL;
    } else if (result == 0) {
        return NULL;
    }

    if (FD_ISSET(socket, &read_fds)) {
        while ((bytes_received = recv(socket, buffer, sizeof(buffer), 0)) > 0) {
            buffer[bytes_received] = '\0';
            data->data = realloc(data->data, data->size + bytes_received + 1);
            memcpy(data->data + data->size, buffer, bytes_received);
            data->size += bytes_received;
            data->data[data->size] = '\0';
            if (bytes_received < 1024) {
                break;
            }
        }
        return data;
    }

    return NULL;
}

struct ip setPASV(int sfd) {
    char buffer[1024] = {0};
    char pasv[1024] = "pasv\r\n";
    send(sfd, pasv, strlen(pasv), 0);
    int bytes = recv(sfd, buffer, 1024, 0);
    buffer[bytes] = 0;
    if (strncmp(buffer, "227", 3) != 0) {
        bytes = recv(sfd, buffer, 1024, 0);
        buffer[bytes] = 0;
    }
    char *p = strstr(buffer, "(");
    char *q = strstr(buffer, ")");
    *q = 0;
    p++;
    char *token = strtok(p, ",");
    int ip[4];
    int port[2];
    int i = 0;
    while (token != NULL) {
        if (i < 4) {
            ip[i] = atoi(token);
        } else {
            port[i - 4] = atoi(token);
        }
        token = strtok(NULL, ",");
        i++;
    }
    char ip_str[1024] = {0};
    sprintf(ip_str, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    int port_num = port[0] * 256 + port[1];
    return create_ip(ip_str, port_num);
}

char *get_current_dir(int sfd) {
    char *dir = malloc(1024);
    char buffer[1024] = {0};
    char pwd[1024] = "pwd\r\n";
    send(sfd, pwd, strlen(pwd), 0);
    int bytes = recv(sfd, buffer, 1024, 0);
    buffer[bytes] = 0;
    if (strncmp(buffer, "257", 3) != 0) {
        bytes = recv(sfd, buffer, 1024, 0);
        buffer[bytes] = 0;
    }
    char *p = strstr(buffer, "\"");
    p++;
    char *q = strstr(p, "\"");
    *q = 0;
    sprintf(dir, "%s", p);
    return dir;
}

