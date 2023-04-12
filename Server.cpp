#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <vector>

#include "datamanage.h"
#include "UserClass.h"


using namespace std;

#define SERV_PORT 9676
const int MAXFDSIZE  = 1024;
char BUF[1024];
void sys_err(const char *str)
{
    perror(str);
    exit(1);
}



int main()
{
    //
    int cnt_msg = -1;

    unordered_map<string, User> Users;
    unordered_map<int, string> ufds;
    // select
    int listenfd, maxfd, sock_fd;
    int clientfds[MAXFDSIZE];

    struct sockaddr_in serv_addr, clie_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1)
    {
        sys_err("socket error");
    }

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(listenfd, 128);

    maxfd = listenfd;
    fd_set  rset, allset;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for(int i = 0; i < MAXFDSIZE; i++)
    {
        clientfds[i] = -1;
    }

    int maxIndex = -1;
    clientfds[++maxIndex] = listenfd;

    while(1)
    {
        rset = allset;
        int nready = select(maxfd+1, &rset, NULL, NULL, NULL);

        if(nready < 0)      sys_err("select error");
        if(nready == 0)     continue;
        if(nready > 0)
        {
            if(FD_ISSET(listenfd, &rset))
            {
                socklen_t clie_addr_len = sizeof(clie_addr);
                int client_fd = accept(listenfd,(struct sockaddr *)&clie_addr, &clie_addr_len);
                if(client_fd < 0)   sys_err("accept error");

                int i;
                for(i = 0; i < MAXFDSIZE; i++)
                {
                    if(clientfds[i] < 0)
                    {
                        clientfds[i] = client_fd;
                        FD_SET(client_fd, &allset);
                        if(maxfd < client_fd)
                            maxfd = client_fd;
                        if(i > maxIndex)
                            maxIndex = i;                            
                        break;
                    }
                }

                if(i == MAXFDSIZE)
                {
                    printf("too many clients\n");
                    exit(1);
                }
                
                if(--nready == 0)
                    continue;                
            }

            for(int i = 0; i <= maxIndex; i++)
            {
                int n;
                if((sock_fd = clientfds[i]) < 0)    continue;

                if(FD_ISSET(clientfds[i], &rset))
                {
                    RECVD_MSG msgFromClient;
                    int a = 2;
                    a++;
                    n = secure_recv(sock_fd, &msgFromClient);
                    // n = read(sock_fd, BUF, sizeof(BUF));
                    if(n == 0)
                    {
                        close(sock_fd);
                        clientfds[i] = -1;
                        if(ufds.find(sock_fd) != ufds.end())   ufds.erase(sock_fd);
                        FD_CLR(sock_fd, &allset);
                    }
                    else if(n > 0)
                    {
                        string s1(msgFromClient.data1);
                        string s2(msgFromClient.data2);
                        // mp[s1] = s2;
                        // cout << msgFromClient.data1 << endl;
                        // cout << msgFromClient.data2 << endl;
                        ServerDealMasseage(&msgFromClient, Users, sock_fd, ufds);
                    }


                    if(--nready == 0)
                        break;
                }
            }
        }
    }
}