#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "handler.h"
#include "ftp.h"
#include "connect.h"

int main() {
    // đăng ký các hàm xử lý signal
    signal(SIGINT, signal_handler);
    signal(SIGCHLD, signal_handler);

    // kết nối đến server
    int sfd = connect_tcp(create_ip("127.0.0.1", 21));

    char *data, input[512] = {0}, cmd[1024] = {0};
    // bỏ qua thông báo server
    recv_data(sfd);

    // đăng nhập
    printf("FTP Client\n");

    // nhập username
    printf("Input username: ");

    fgets(input, 512, stdin);
    if (input[strlen(input) - 1] == '\n')
        input[strlen(input) - 1] = '\0';

    // lệnh gửi username của ftp là "user <username>"
    sprintf(cmd, "user %s\r\n", input);
    send(sfd, cmd, strlen(cmd), 0);
    recv_data(sfd);

    // nhập password
    printf("Input password: ");

    fgets(input, 512, stdin);
    if (input[strlen(input) - 1] == '\n')
        input[strlen(input) - 1] = '\0';

    // lệnh gửi password của ftp là "pass <password>"
    sprintf(cmd, "pass %s\r\n", input);
    send(sfd, cmd, strlen(cmd), 0);

    // cấu trúc data trả về của ftp là "230 Login successful"
    data = recv_data(sfd)->data;
    if (strstr(data, "230") == NULL) {
        printf("Login failed\n");
        exit(0);
    } else {
        printf("Login successful\n");
    }

    // khi login thành công, chương trình sẽ chạy trong vòng lặp này
    while (1) {
        // bật chế độ passive cuả ftp bằng lệnh "pasv"
        // tác dụng của pasv là để tránh bị firewall chặn
        // khi gửi lệnh pasv thì server sẽ trả về ip và port để kết nối đến theo cấu trúc "227 Entering Passive Mode (ip1,ip2,ip3,ip4,port1,port2)"
        // ip1.ip2.ip3.ip4 là ip của server, port1*256+port2 là port của server
        // sau đó lấy ip và port của server để kết nối đến
        struct ip temp_ip = setPASV(sfd);
        char command[1024] = {0};

        // hiển thị menu
        printf("Menu (Current directory: %s):\n", get_current_dir(sfd));
        printf("1. Get file list\n");
        printf("2. Change directory\n");
        printf("3. Download files\n");
        printf("4. Upload files\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        int choice;

        // nhập lựa chọn
        // %*c để bỏ qua ký tự '\n' sau khi nhập
        scanf("%d%*c", &choice);
        switch (choice) {
            case 1:
                // lệnh "list" của ftp để lấy danh sách file
                sprintf(command, "list\r\n");

                // gửi lệnh
                send(sfd, command, strlen(command), 0);

                // tạo tiến trình con để kết nối đến server qua ip và port lấy được
                // từ lệnh "pasv"
                // fork trả về 0 nếu là tiến trình con
                if (fork() == 0) {
                    // kết nối đến server qua ip và port lấy được từ lệnh "pasv"
                    int cfd = connect_tcp(temp_ip);
                    data = recv_data(cfd)->data;
                    printf("List of files:\n");
                    char *line, *name;
                    // các dòng file được phân cách bởi ký tự '\n'
                    line = strtok(data, "\n");
                    while (line != NULL) {
                        // tên file được lấy từ vị trí thứ 56
                        name = line + 56;

                        // thư mục sẽ bắt đầu bằng ký tự 'd' hoặc 't'
                        // in màu xanh cho thư mục
                        // bắt đầu màu xanh bằng cách in "\033[38;5;2m"
                        // kết thúc màu xanh bằng cách in "\033[0m"
                        if (line[0] == 't' || line[0] == 'd')
                            printf("\033[38;5;2m%s\033[0m\n", name);
                        else
                            printf("%s\n", name);

                        // lấy dòng tiếp theo
                        line = strtok(NULL, "\n");
                    }
                    exit(0);
                }

                // bỏ qua thông báo gửi thành công của ftp
                recv_data(sfd);
                break;
            case 2:
                // nhập tên thư mục
                printf("Enter directory name: ");
                char dir[512] = {0};
                fgets(dir, 512, stdin);
                sprintf(command, "cwd %s\r\n", dir);
                send(sfd, command, strlen(command), 0);
                // kiểm tra thông báo gửi thành công của ftp
                // nếu là "550" thì thư mục không tồn tại
                // nếu là "250" thì thư mục tồn tại
                data = recv_data(sfd)->data;
                if (strstr(data, "550") == NULL)
                    printf("Directory changed successfully\n");
                else
                    printf("Directory change failed\n");
                break;
            case 3:
                // bất cứ khi nào gửi lệnh "retr" thì phải bật chế độ binary
                // lệnh "type i" để chuyển sang chế độ binary
                sprintf(command, "type i\r\n");
                send(sfd, command, strlen(command), 0);
                recv_data(sfd);

                // nhập tên file
                printf("Enter file name: ");
                char file[512] = {0};
                fgets(file, 512, stdin);
                if (file[strlen(file) - 1] == '\n') {
                    file[strlen(file) - 1] = '\0';
                }

                // gửi lệnh "retr" để yêu cầu server gửi file
                sprintf(command, "retr %s\r\n", file);
                send(sfd, command, strlen(command), 0);
                if (fork() == 0) {

                    // tạo tiến trình con để kết nối đến server qua ip và port lấy được
                    int cfd = connect_tcp(temp_ip);
                    struct data *temp = recv_data(cfd);

                    // tách tên file từ đường dẫn ví dụ: /home/user/file.txt -> file.txt
                    while (strstr(file, "/") != NULL) {
                        sprintf(file, "%s", strstr(file, "/") + 1);
                    }

                    // ghi data vào file
                    FILE *fp = fopen(file, "wb");
                    fwrite(temp->data, 1, temp->size, fp);
                    fclose(fp);
                    exit(0);
                }

                // lấy thông báo gửi thành công của ftp
                data = recv_data(sfd)->data;
                // nếu là "550" thì file không tồn tại
                // nếu là "150" thì đã download thành công
                if (strstr(data, "550") != NULL) {
                    printf("File not found\n");
                    remove(file);
                } else if (strstr(data, "150") != NULL) {
                    printf("File downloaded\n");
                }
                break;
            case 4:
                // bất cứ khi nào gửi lệnh "stor" thì phải bật chế độ binary
                // lệnh "type i" để chuyển sang chế độ binary
                sprintf(command, "type i\r\n");
                send(sfd, command, strlen(command), 0);
                recv_data(sfd);
                printf("Enter file name: ");
                char file2[512] = {0};
                fgets(file2, 512, stdin);
                if (file2[strlen(file2) - 1] == '\n') {
                    file2[strlen(file2) - 1] = '\0';
                }

                // kiểm tra file có tồn tại không
                FILE *tmp = fopen(file2, "rb");
                if (tmp == NULL) {
                    printf("File not found\n");
                    break;
                }
                fclose(tmp);

                // gửi lệnh "stor <tên file>" để yêu cầu server nhận file
                sprintf(command, "stor %s\r\n", file2);
                send(sfd, command, strlen(command), 0);
                if (fork() == 0) {
                    // tạo tiến trình con để kết nối đến server qua ip và port lấy được
                    int cfd = connect_tcp(temp_ip);

                    // đọc file dưới dạng binary là chữ b ở trong rb
                    FILE *fp = fopen(file2, "rb");
                    fseek(fp, 0, SEEK_END);
                    int size = ftell(fp);
                    fseek(fp, 0, SEEK_SET);
                    char *buffer = malloc(size);
                    fread(buffer, 1, size, fp);
                    fclose(fp);

                    // gửi file
                    send(cfd, buffer, size, 0);
                    exit(0);
                }

                // lấy thông báo gửi thành công của ftp
                data = recv_data(sfd)->data;
                // nếu là "150" thì đã upload thành công
                if (strstr(data, "150") != NULL) {
                    printf("File uploaded\n");
                }
                break;
            case 5:
                // thoát khỏi chương trình
                exit(0);
            default:
                printf("Invalid choice\n");
                break;
        }
    }
}