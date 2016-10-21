//server.c
#include <pthread.h>
#include <sys/socket.h>
//#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <list>

using namespace std;

bool debug;

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

    void send(char *message)
    {
        int len = strlen(message);
        for(list<int>::iterator it = userSocket.begin(); it != userSocket.end(); ++it)
            write(*it, message, len);
    }

private:
    list<int>userSocket;  //store the client's socket;
}

void init(int port, int maxClnt)
{
    int serv_sock,clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    int clnt_addr_sz;

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(port);

    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
        error("bind() error!");
    if(listen(serv_sock, maxClnt) == -1)
        error("listen() error!");

    if(debug)
    {
        //daemon_init;

    }

}

int main(int argc, char *argv[])
{
    return 0;
}
