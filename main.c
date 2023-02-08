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
    signal(SIGINT, signal_handler);
    signal(SIGCHLD, signal_handler);
    int sfd = connect_tcp(create_ip("127.0.0.1", 21));

    char *data, input[512] = {0}, cmd[1024] = {0};
    recv_data(sfd);

    printf("FTP Client\n");
    printf("Input username: ");
    fgets(input, 1024, stdin);
    if (input[strlen(input) - 1] == '\n')
        input[strlen(input) - 1] = '\0';
    sprintf(cmd, "user %s\r\n", input);
    send(sfd, cmd, strlen(cmd), 0);
    recv_data(sfd);
    printf("Input password: ");

    fgets(input, 1024, stdin);
    if (input[strlen(input) - 1] == '\n')
        input[strlen(input) - 1] = '\0';
    sprintf(cmd, "pass %s\r\n", input);
    send(sfd, cmd, strlen(cmd), 0);
    data = recv_data(sfd)->data;
    if (strstr(data, "230") == NULL) {
        printf("Login failed\n");
        exit(0);
    } else {
        printf("Login successful\n");
    }

    while (1) {
        struct ip temp_ip = setPASV(sfd);
        char command[1024] = {0};
        printf("Menu (Current directory: %s):\n", get_current_dir(sfd));
        printf("1. Get file list\n");
        printf("2. Change directory\n");
        printf("3. Download files\n");
        printf("4. Upload files\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        int choice;
        scanf("%d%*c", &choice);
        switch (choice) {
            case 1:
                sprintf(command, "list\r\n");
                send(sfd, command, strlen(command), 0);
                if (fork() == 0) {
                    int cfd = connect_tcp(temp_ip);
                    data = recv_data(cfd)->data;
                    printf("List of files:\n");
                    char *line, *name;
                    line = strtok(data, "\n");
                    while (line != NULL) {
                        name = line + 56;
                        if (line[0] == 't' || line[0] == 'd')
                            printf("\033[38;5;2m%s\033[0m\n", name);
                        else
                            printf("%s\n", name);
                        line = strtok(NULL, "\n");
                    }
                    exit(0);
                }
                recv_data(sfd);
                break;
            case 2:
                printf("Enter directory name: ");
                char dir[512] = {0};
                fgets(dir, 512, stdin);
                sprintf(command, "cwd %s\r\n", dir);
                send(sfd, command, strlen(command), 0);
                data = recv_data(sfd)->data;
                if (strstr(data, "550") == NULL)
                    printf("Directory changed successfully\n");
                else
                    printf("Directory change failed\n");
                break;
            case 3:
                sprintf(command, "type i\r\n");
                send(sfd, command, strlen(command), 0);
                recv_data(sfd);
                printf("Enter file name: ");
                char file[512] = {0};
                fgets(file, 512, stdin);
                if (file[strlen(file) - 1] == '\n') {
                    file[strlen(file) - 1] = '\0';
                }
                sprintf(command, "retr %s\r\n", file);
                send(sfd, command, strlen(command), 0);
                if (fork() == 0) {
                    int cfd = connect_tcp(temp_ip);
                    struct data *temp = recv_data(cfd);
                    while (strstr(file, "/") != NULL) {
                        sprintf(file, "%s", strstr(file, "/") + 1);
                    }
                    FILE *fp = fopen(file, "wb");
                    fwrite(temp->data, 1, temp->size, fp);
                    fclose(fp);
                    exit(0);
                }
                data = recv_data(sfd)->data;
                if (strstr(data, "550") != NULL) {
                    printf("File not found\n");
                    remove(file);
                } else if (strstr(data, "150") != NULL) {
                    printf("File downloaded\n");
                }
                break;
            case 4:
                // Upload file
                sprintf(command, "type i\r\n");
                send(sfd, command, strlen(command), 0);
                recv_data(sfd);
                printf("Enter file name: ");
                char file2[512] = {0};
                fgets(file2, 512, stdin);
                if (file2[strlen(file2) - 1] == '\n') {
                    file2[strlen(file2) - 1] = '\0';
                }

                if (fopen(file2, "rb") == NULL) {
                    printf("File not found\n");
                    break;
                }
                sprintf(command, "stor %s\r\n", file2);
                send(sfd, command, strlen(command), 0);
                if (fork() == 0) {
                    int cfd = connect_tcp(temp_ip);
                    FILE *fp = fopen(file2, "rb");
                    fseek(fp, 0, SEEK_END);
                    int size = ftell(fp);
                    fseek(fp, 0, SEEK_SET);
                    char *buffer = malloc(size);
                    fread(buffer, 1, size, fp);
                    fclose(fp);
                    send(cfd, buffer, size, 0);
                    exit(0);
                }
                data = recv_data(sfd)->data;
                if (strstr(data, "150") != NULL) {
                    printf("File uploaded\n");
                }
                break;
            case 5:
                exit(0);
            default:
                printf("Invalid choice\n");
                break;
        }
    }
}