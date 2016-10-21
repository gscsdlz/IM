//server.c
#include <iostream>
#include <pthread.h>
#include <sys/socket.h>
//#include <mysql/mysql.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <list>

using namespace std;

bool debug;
pthread_mutex_t mtx;
class Group
{
public:
    void join(int socket_clnt)
    {
        userSocket.push_back(socket_clnt);
    }

    void leave(int socket_clnt)
    {
        userSocket.remove(socket_clnt);
        close(socket_clnt);
    }

    void sendInfo(char *message)
    {
        int len = strlen(message);
        for(list<int>::iterator it = userSocket.begin(); it != userSocket.end(); ++it)
            write(*it, message, len);
    }

    int getGid()
    {
        return gid;
    }

private:
    list<int>userSocket;  //store the client's socket;
    int gid;
};
map<int, Group *>groupMap;

void error(char *message)
{
    fputs(message,stderr);
    fputc('\n', stderr);
    exit(1);
}

void init(int port, int maxClnt)
{
    int serv_sock;
    struct sockaddr_in serv_adr;


    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(port);

    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
        error("bind() error!");
    if(listen(serv_sock, maxClnt) == -1)
        error("listen() error!");

    if(!debug)
    {
        //daemon_init;

    }
    pthread_mutex_init(&mtx, NULL);
}

void sendInfo(char *message, int clnt_sock)
{
    if(m[clnt_sock] != NULL)
        int gid = m[clnt_sock]->sendInfo(message[]);

}

void login(char *message, int clnt_sock)
{
    //message type Lusername:password
    int len = strlen(message);
    char username[100];
    char password[100];
    int p1 = 0, p2 = 0;
    bool flag = false;
    for(int i = 1; i < len; ++i)
    {
        if(message[i] == ':')
        {
            flag = true;
            continue;
        }
        if(flag)
            password[p2++] = message[i];
        else
            username[p1++] = message[i];
    }
    /////////////////////////////
    //mysql SELECT;
    ////////////////////////////
    int gid = 1;  //from Database
    if(groupMap[gid] == NULL)
        groupMap[gid] = new Group();

    pthread_mutex_lock(&mtx);
    groupMap[gid]->join(clnt_sock);
    pthread_mutex_unlock(&mtx);
}

void sendMsg(char * message, int clnt_sock)
{
    write(clnt_sock, message, strlen(message));
}

void * readInfo(void *arg)
{
    int clnt_sock = *((int *)arg);
    int str_len = 0;
    char msgBuff[BUF_SIZE];
    memset(msgBuff, 0, sizeof(msgBuff));

    do
    {
        int recv_len = read(clnt_sock, &msgBuff[str_len], BUF_SIZE - str_len);
        if(recv_len == -1)
            error("read() error!");
        str_len += recv_len;
    }
    while(msgBuff[str_len - 1] != '\0');

    if(msgBuff[0] == 'L') // mains Login
        login(msgBuff, clnt_sock);
    else if(msgBuff[0] == 'S')
        sendMsg(msgBuff);
}

void keepConnect()
{
    struct sockaddr_in clnt_adr;
    pthread_t tid;
    while(1)
    {
        int clnt_adr_sz = sizeof(clnt_adr);
        int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
        pthread_create(&tid, NULL, readInfo, (void *)&clnt_sock);
        pthread_detach(tid);
    }
}

int main(int argc, char *argv[])
{
    if(argc < 2 /* or maybe other number*/)
        error("123");
    int port = atoi(argv[1]);
    int maxClnt = atoi(argv[2]);
    if(argc == 4 && argv[3][0] == 'D')
        debug = true;
    init(port, maxClnt);

    return 0;
}
