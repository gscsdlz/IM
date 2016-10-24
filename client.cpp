//client.c
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 1024

char fuck;

void error(char *message)
{
    fputs(message, stderr);
    fputs("\n", stderr);
    exit(1);
}

void *readInfo(void *arg)
{
    int sock = *((int *)arg);
    char msgBuff[BUF_SIZE] = {0};
    int recv_len;
    int str_len = 0;
    int Ca = 1;
    while((recv_len = read(sock, &msgBuff[str_len], BUF_SIZE)) != 0)
    {
        if(recv_len == -1)
            error("read() error!");
        str_len += recv_len;
        if(msgBuff[str_len] == '\0') //recv all the info
        {
            printf("%s\n", msgBuff);
            str_len = 0;
            memset(msgBuff, 0 ,sizeof(msgBuff));
        }
    }
}

void *writeInfo(void *arg)
{
    char msgBuff[BUF_SIZE];
    int sock = *((int *)arg);
    for(int i = 0; i < 40; ++i)
        msgBuff[i] = rand() % 10 + '0';
    msgBuff[40] = ':';
    for(int i = 41; i <= 45; ++i)
        msgBuff[i] = '1';
    msgBuff[46] = ':';
    srand((unsigned)time(NULL));
    int option = rand() % 4;
    msgBuff[47] = fuck;
    switch (option)
    {
        case 0:
            strcat(&msgBuff[48], "Hello");
            break;
        case 1:
            strcat(&msgBuff[48], "My Name");
            break;
        case 2:
            strcat(&msgBuff[48], "Who are you");
            break;
        case 3:
            strcat(&msgBuff[48], "Hi");
            break;
    }
    while(1)
    {
        sleep(5);
        write(sock, msgBuff, sizeof(msgBuff));
    }
}

void connect(char *ip, int port)
{
    int sock;
    pthread_t pidr, pidw;
    struct sockaddr_in serv_addr;
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1)
        error("socket() error");
    printf("%s\n", ip);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    if(connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
        error("connect() error");

    pthread_create(&pidr, NULL, readInfo, (void *)&sock);
    pthread_create(&pidw, NULL, writeInfo, (void *)&sock);
    pthread_join(pidr, NULL);
    pthread_join(pidw, NULL);
}

int main(int argc, char *argv[])
{
    if(argc != 4)
        printf("Usage: ./client HOST port type");
    else
    {
        fuck = argv[3][0];
        connect(argv[1], atoi(argv[2]));
    }

}
