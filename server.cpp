//server.c
#include <iostream>
#include <pthread.h>
#include <sys/socket.h>
//#include <mysql/mysql.h>
#include <map>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <list>

#define BUF_SIZE 1024

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
map<int, Group *>groupMap;  //int mains the gid

void error(char *message)
{
    fputs(message,stderr);
    fputc('\n', stderr);
    exit(1);
}

int init(int port, int maxClnt)
{
    int serv_sock;
    struct sockaddr_in serv_adr;


    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
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
    return serv_sock;
}

void sendErrInfo(char *message, int clnt_sock)
{
    
}

int checkLogin(char *message)
{
    //message type sha1(40)
  

    /////////////////////////////
    //mysql SELECT;
    ////////////////////////////
    int gid = 10000;  //from Database
    pthread_mutex_lock(&mtx);
    if(groupMap[gid] == NULL)
        groupMap[gid] = new Group();
    groupMap[gid]->join(clnt_sock);
    pthread_mutex_unlock(&mtx);
    return gid;
}

void * readInfo(void *arg)
{
    int clnt_sock = *((int *)arg);
    int str_len = 0;
    int gid = 0;
    char msgBuff[BUF_SIZE];
    memset(msgBuff, 0, sizeof(msgBuff));
    int recv_len;

    while((recv_len = read(clnt_sock, &msgBuff[str_len], BUF_SIZE)) != 0)
    {
        if(recv_len == -1)
            error("read() error!");
        str_len += recv_len;
        if(msgBuff[str_len - 1] == '\0') //recv all the info
        {
            if(gid = checkLogin(msgBuff))
                sendInfo(msgBuff, clnt_sock);
            memset(msgBuff, '\0', sizeof(msgBuff));
            str_len = 0;
        }
    }
    pthread_mutex_lock(&mtx);
    if(gid)
        groupMap[gid]->leave(clnt_sock);
    if(groupMap[gid]->size() == 0)
    {
        delete groupMap[gid];
        groupMap[gid] = NULL;
    }     
    pthread_mutex_unlock(&mtx);
    close(clnt_sock);
}

void keepConnect(int serv_sock)
{
    struct sockaddr_in clnt_adr;
    pthread_t tid;
    while(1)
    {
        socklen_t clnt_adr_sz = sizeof(clnt_adr);
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
    int serv_sock = init(port, maxClnt);
    keepConnect(serv_sock);
    return 0;
}
