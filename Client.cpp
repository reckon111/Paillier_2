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


#include "paillier.h"
#include "datamanage.h"

#define SERV_PORT 9676

using namespace std;

const int MAXFDSIZE  = 1024;
char BUF[1024];
void sys_err(const char *str)
{
    perror(str);
    exit(1);
}
int conToServer()
{
    int sock_cli = socket(AF_INET,SOCK_STREAM, 0);

    ///定义sockaddr_in
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT );  ///服务器端口
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");  ///服务器ip

    ///连接服务器，成功返回0，错误返回-1
    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect");
        exit(1);
    }
	printf("连接服务器成功\n");

    return sock_cli;
}
int main()
{

    mpz_t pk_n, pk_g, pk_hs, sk_lambda, sk_mu, encipher_msg1, encipher_msg2, rel_mpz, npow2, test_enmsg;

    // 初始化
    mpz_init(pk_n);
    mpz_init(pk_g);
    mpz_init(pk_hs);
    mpz_init(sk_lambda);
    mpz_init(sk_mu);
    mpz_init(encipher_msg1);
    mpz_init(encipher_msg2);
    mpz_init(rel_mpz);
    mpz_init(npow2);
    mpz_init(test_enmsg);

    // 客户端产生密钥
    KEY my_key;
    

    int sock_cli = conToServer();
    if(sock_cli < 0)
    {
        cout << "erro" << endl;
        exit(1);
    }

    RECVD_MSG MsgFromServer;
    
    //_____________________用户交互_______________
    printf("输入0:注册\t输入1:登陆\t输入2:发送密文\t输入3:发送公钥\t输入4:发送求和请求\n");
    while((fgets(BUF, sizeof(BUF), stdin)) != NULL)
    {    
        if(strncmp(BUF, "0", 1) == 0)   // 0:注册 发送用户名 及 密码
        {
            char uername[10], passwd[20];
            printf("输入用户名:\n");
            scanf("%s", uername);
            printf("输入密码:\n");
            scanf("%s", passwd);

            char* send_data = encode_msg(0, uername, passwd); 
            secure_send(sock_cli, send_data);
            free(send_data);
            read(sock_cli, BUF, sizeof(BUF));
            fputs(BUF, stdout);
        }

        if(strncmp(BUF, "1", 1) == 0)   // 1:登陆
        {
            char uername[10], passwd[20];
            printf("输入用户名:\n");
            scanf("%s", uername);
            printf("输入密码:\n");
            scanf("%s", passwd);

            char* send_data = encode_msg(1, uername, passwd); 
            secure_send(sock_cli, send_data);
            printf("发送数据成功\n");
            free(send_data);
            read(sock_cli, BUF, sizeof(BUF));
            fputs(BUF, stdout);
        }

        if(strncmp(BUF, "2", 1) == 0)       // 2:发送密文
        {
            char path[30];
            printf("待加密文件路径:\n");
            scanf("%s", path);
            vector<char*> v;
            encipher_file(path, v, my_key);
            for(int i=0; i<v.size(); i++)
            {
                secure_send(sock_cli, v[i]);
                free(v[i]);
                read(sock_cli, BUF, sizeof(BUF));
                fputs(BUF, stdout);
            }
        }

        if(strncmp(BUF, "3", 1) == 0)   // 3:发送公钥
        {
            char s1[] = "3";
            char strk_n[KEYSIZE / 2 + 10];
            mpz_get_str(strk_n, 16, my_key.pk_n);
            // printf("strk_n%s\n:", strk_n);
            char* send_data = encode_msg(3, strk_n, strk_n);
            secure_send(sock_cli, send_data);
            printf("发送公钥成功\n");
            free(send_data);

            read(sock_cli, BUF, sizeof(BUF));
            fputs(BUF, stdout);
        }

        if(strncmp(BUF, "4", 1) == 0)   // 4:发送求和请求
        {
            char s1[] = "4";
            
            char* send_data = encode_msg(4, s1, s1); 
            secure_send(sock_cli, send_data);
            printf("发送请求成功\n");
            
            if(send_data)
                free(send_data);

            RECVD_MSG msg;
            secure_recv(sock_cli, &msg);

            mpz_t rel_mpz;
            mpz_init(rel_mpz);
            mpz_set_str(rel_mpz, msg.data1, BASE);
            unsigned long rel = decipher_G(rel_mpz, my_key.pk_n, my_key.sk_lambda, my_key.sk_mu);
            printf("rel:%lu\n", rel);
        }

    }


    
}