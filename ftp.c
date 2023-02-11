#include "ftp.h"

struct data *recv_data(int socket) {
    // khai báo cấu trúc data để lưu dữ liệu trả về
    struct data *data = malloc(sizeof(struct data));
    data->data = malloc(1);
    data->size = 0;

    fd_set read_fds;
    struct timeval timeout;
    int bytes_received;

    // xóa các socket trong set
    FD_ZERO(&read_fds);

    // thêm server socket vào set
    FD_SET(socket, &read_fds);

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    char buffer[1024] = {0};

    // đợi các kết nối đến server
    // nếu không có kết nối nào đến trong 5s thì trả về NULL
    int result = select(socket + 1, &read_fds, NULL, NULL, &timeout);
    if (result == -1) {
        perror("select");
        return NULL;
    } else if (result == 0) {
        return NULL;
    }

    // nếu có kết nối đến
    if (FD_ISSET(socket, &read_fds)) {
        // sử dụng vòng lặp để nhận dữ liệu
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


// gửi lệnh PASV tới server để bật chế độ passive
// trả về cấu trúc ip chứa ip và port của server
struct ip setPASV(int sfd) {
    // gửi lệnh PASV
    char buffer[1024] = {0};
    char pasv[1024] = "pasv\r\n";
    send(sfd, pasv, strlen(pasv), 0);
    int bytes = recv(sfd, buffer, 1024, 0);
    buffer[bytes] = 0;

    // kết quả trả về của lệnh PASV là "227 Entering Passive Mode (ip1,ip2,ip3,ip4,port1,port2)"
    if (strncmp(buffer, "227", 3) != 0) {
        bytes = recv(sfd, buffer, 1024, 0);
        buffer[bytes] = 0;
    }
    // lấy vị trí của dấu "(" và ")" sau đó set ký tự ")" thành ký tự null
    // khi đó chuỗi buffer sẽ chỉ còn lại "ip1,ip2,ip3,ip4,port1,port2"
    char *p = strstr(buffer, "(");
    char *q = strstr(buffer, ")");
    *q = 0;
    p++;

    //strtok để tách chuỗi buffer thành các chuỗi con cách nhau bởi dấu ","
    char *token = strtok(p, ",");
    int ip[4];
    int port[2];
    int i = 0;
    while (token != NULL) {
        if (i < 4) {
            // chuyển chuỗi con thành số nguyên
            ip[i] = atoi(token);
        } else {
            port[i - 4] = atoi(token);
        }

        // lấy chuỗi con tiếp theo
        token = strtok(NULL, ",");
        i++;
    }

    // tạo chuỗi ip_str từ các số nguyên ip[4]
    char ip_str[1024] = {0};
    // ip_str = "ip1.ip2.ip3.ip4"
    // 127.0.0.1
    sprintf(ip_str, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    // port = port1 * 256 + port2
    int port_num = port[0] * 256 + port[1];

    // trả về cấu trúc ip
    return create_ip(ip_str, port_num);
}

// gửi lệnh pwd tới server để lấy thư mục hiện tại
char *get_current_dir(int sfd) {
    char *dir = malloc(1024);
    char buffer[1024] = {0};
    char pwd[1024] = "pwd\r\n";
    send(sfd, pwd, strlen(pwd), 0);
    int bytes = recv(sfd, buffer, 1024, 0);
    buffer[bytes] = 0;

    // kết quả trả về của lệnh pwd là "257 "current dir" is current directory"
    if (strncmp(buffer, "257", 3) != 0) {
        bytes = recv(sfd, buffer, 1024, 0);
        buffer[bytes] = 0;
    }
    // lấy vị trí của dấu " đầu tiên
    char *p = strstr(buffer, "\"");
    p++;
    // lấy vị trí của dấu " thứ 2 và set ký tự " thành ký tự null
    char *q = strstr(p, "\"");
    *q = 0;
    // dir = /home/test
    sprintf(dir, "%s", p);
    return dir;
}

