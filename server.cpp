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
map<int, int>socketMap;  //First int mains socket, second int mains gid;

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

void sendInfo(char *message, int clnt_sock)
{
    int gid = socketMap[clnt_sock];
    if(gid == 0)
    {
        char errorMsg[] = "No such group!\n";
        write(clnt_sock, errorMsg, sizeof(errorMsg));
    }
    else
    {
        groupMap[gid]->sendInfo(message);
    }
}

void login(char *message, int clnt_sock)
{
    //message type Lusername:password:gid
    int len = strlen(message);
    char username[100];
    char password[100];
    char gidString[100];
    int p1 = 0, p2 = 0, p3 = 0;
    memset(username, '\0', sizeof(username));
    memset(password, '\0', sizeof(password));
    memset(gidString, '\0', sizeof(gidString));

    int flag = 0;
    for(int i = 1; i < len; ++i)
    {
        if(message[i] == ':')
        {
            flag++;
            continue;
        }
        if(flag == 0)
            username[p1++] = message[i];
        else if(flag == 1)
            password[p2++] = message[i];
        else
            gidString[p3++] = message[i];
    }

    /////////////////////////////
    //mysql SELECT;
    ////////////////////////////
    int gid = atoi(gidString);  //from Database

    pthread_mutex_lock(&mtx);
    socketMap[clnt_sock] = gid;
    if(groupMap[gid] == NULL)
        groupMap[gid] = new Group();
    groupMap[gid]->join(clnt_sock);
    pthread_mutex_unlock(&mtx);
}

void * readInfo(void *arg)
{
    int clnt_sock = *((int *)arg);
    int str_len = 0;
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
            if(msgBuff[0] == 'L') // mains Login
                login(msgBuff, clnt_sock);
            else if(msgBuff[0] == 'S')
                sendInfo(msgBuff, clnt_sock);

            memset(msgBuff, '\0', sizeof(msgBuff));
            str_len = 0;
        }
    }
    if(socketMap[clnt_sock] != 0)
        groupMap[socketMap[clnt_sock]]->leave(clnt_sock);
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
