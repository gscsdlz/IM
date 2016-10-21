//client.c
//client.c
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char id_buf[1024];
int jobs[20];
int fp;

void error_handing(char *message)
{
    fputs(message, stderr);
    fputs("\n", stderr);
    exit(1);
}

int connect(int port, char *ip)
{
    int sock;

    struct sockaddr_in serv_addr;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1)
        error_handing("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    if(connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
        error("connect() error");
    else
        puts("Connect....");

    int str_len = 0;
    while(id_buf[str_len - 1] != 'S')
    {
        int recv_cnt = read(sock, &id_buf[str_len], 1024);
        str_len += recv_cnt;
    }
    id_buf[str_len] = 0;
    close(sock);
    puts(id_buf);
    return str_len;
}

int main(int argc, char *argv[])
{
    if(argc )
}
