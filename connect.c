#include "connect.h"

// tạo 1 kết nối tcp tới server qua ip và port
int connect_tcp(struct ip ip) {
    // tạo socket
    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sfd == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(ip.port);
    addr.sin_addr.s_addr = inet_addr(ip.ip);

    // kết nối tới server
    if (connect(sfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("connect");
        return -1;
    }

    return sfd;
}

// tạo struct ip từ chuỗi ip và số port
struct ip create_ip(char *ip_str, int port_num) {
    struct ip cfd;
    strcpy(cfd.ip, ip_str);
    cfd.port = port_num;
    return cfd;
}