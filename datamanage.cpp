#include "datamanage.h"
#include "UserClass.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

int secure_recv(int sock_cli, RECVD_MSG *recvd_msg){

    char recv_buffer[BUFFER_SIZE];
    char data_buffer[5096];
    memset(recv_buffer,0,sizeof(recv_buffer));
    memset(data_buffer,0,sizeof(data_buffer));
    int len = 0, point = 0;
    while(1){
        len = recv(sock_cli, recv_buffer, sizeof(recv_buffer), 0);
        if( len == 0)
        {
            break;
        }

        strncpy(data_buffer+point, recv_buffer, len);
        point += len;
        memset(recv_buffer, 0, sizeof(recv_buffer));

        while(1)
        {
            if(strlen(data_buffer) < HEADER_SIZE)   // 接受数据长度小于消息头
            {
                break;
            }

            char header[HEADER_SIZE+1]; // 消息头
            int type_size = 1, size1, size2;
            char s_t[TYPE_LEN+1], s_size1[MSG1BYTES_LEN+1], s_size2[MSG1BYTES_LEN+1];
            memset(header, 0, sizeof(header));
            memset(s_t, 0, sizeof(s_t));
            memset(s_size1, 0, sizeof(s_size1));
            memset(s_size2, 0, sizeof(s_size2));

            strncpy(header, data_buffer, HEADER_SIZE);
            strncpy(s_t, header, TYPE_LEN);
            strncpy(s_size1, header+TYPE_LEN, MSG1BYTES_LEN);
            strncpy(s_size2, header+TYPE_LEN+MSG1BYTES_LEN, MSG2BYTES_LEN);

            size1 = atoi(s_size1); 
            size2 = atoi(s_size2);
            
            if(strlen(data_buffer) < HEADER_SIZE + size1 + size2) // 接受数据小于消息头+消息体长度
            {
                break;
            }

            char *dmsg1 = (char*)calloc(size1+1, sizeof(char));
            char *dmsg2 = (char*)calloc(size2+1, sizeof(char));
            strncpy(dmsg1, data_buffer+HEADER_SIZE, size1);
            strncpy(dmsg2, data_buffer+HEADER_SIZE+size1, size2);
            recvd_msg->type = atoi(s_t);
            recvd_msg->data1 = dmsg1;
            recvd_msg->data2 = dmsg2;

            memset(data_buffer,0,sizeof(data_buffer));
            return point;
        }

        if(strlen(data_buffer) == 0)
        {
            break;
        }
    }

    return point;
}

void secure_send(int fd, char* send_data){  // 避免数据过长，而数据发送不完整，保证数据能完整传送完
    int len , cnt = 0;
    int size = strlen(send_data);

    while(size > 0){
        if(strlen(send_data+cnt) > 1024)
        {
            len = send(fd, send_data+cnt, 1024, 0);
        }
        else
        {
            len = send(fd, send_data+cnt, strlen(send_data+cnt), 0);
        }
        
        if(len == -1)
        {
            printf("发送错误\n");
            return;
        }

        cnt += len;
        size -= len;
    }    
}

char* encode_msg(int type, char* msg1, char* msg2){ //函数使用结束后需要free
    int header_size;
    int msg1_size = strlen(msg1);
    int msg2_size = strlen(msg2);
    char header[10];
    header_size = snprintf(header, sizeof(header), "%1d%04d%04d", type, msg1_size, msg2_size);
    char *encode_msg = (char*)malloc((header_size+msg1_size+msg2_size+1)*sizeof(char));
    strcpy(encode_msg, header);
    strcat(encode_msg, msg1);
    strcat(encode_msg, msg2);
    return encode_msg;
}
void ServerDealMasseage(RECVD_MSG *recvd_msg, unordered_map<string, User> &Users, int fd, unordered_map<int, string> &ufds)
{
    if(recvd_msg->type == 0)  // 客户注册
    {
        ufds[fd] = string(recvd_msg->data1);
        User user(fd, string(recvd_msg->data1), string(recvd_msg->data2));
        Users[string(recvd_msg->data1)] = user;
        
        if(recvd_msg->data1)
            free(recvd_msg->data1);
        if(recvd_msg->data2)
            free(recvd_msg->data2);
        printf("用户注册成功\n");
        char ss[] = "注册成功\n";
        write(fd, ss, sizeof(ss));
    }

    if(recvd_msg->type == 1)  // 客户登陆
    {
        if(Users.find(string(recvd_msg->data1)) != Users.end())
        {
            
            if(string(recvd_msg->data2) == Users[string(recvd_msg->data1)].GetPasswd())
            {
                ufds[fd] = string(recvd_msg->data1);
                printf("用户登陆成功\n");
            }
            else
            {
                printf("用户密码错误\n");
            }
        }
        else 
        {
            printf("用户不存在\n");
        }
        char ss[] = "登陆成功\n";
        write(fd, ss, sizeof(ss));

        if(recvd_msg->data1)
            free(recvd_msg->data1);
        if(recvd_msg->data2)
            free(recvd_msg->data2);
    }

    if(recvd_msg->type == 2)  // 客户发送密文
    {
        Users[ufds[fd]].StoreEncidata(recvd_msg->data1, recvd_msg->data2);

        if(recvd_msg->data1)
            free(recvd_msg->data1);
        if(recvd_msg->data2)
            free(recvd_msg->data2);

        printf("接受密文成功\n");

        char ss[] = "ok\n";
        write(fd, ss, sizeof(ss));
    }

    if(recvd_msg->type == 3)  // 客户发送公钥
    {
        Users[ufds[fd]].Record_pk(recvd_msg->data1);
        // std::cout << Users[ufds[fd]].Get_pk() << endl;
        printf("接受公钥成功\n");

        char ss[] = "ok\n";
        write(fd, ss, sizeof(ss));

        if(recvd_msg->data1)
            free(recvd_msg->data1);
        if(recvd_msg->data2)
            free(recvd_msg->data2);   
    
    }

    if(recvd_msg->type == 4)  // 密文求和
    {
        // mpz_mod();
        mpz_t rel, a, npow2;
        mpz_init_set_ui(rel, 1);
        mpz_init(a);
        mpz_init(npow2);
        // s.c_str()
        mpz_set_str(npow2, Users[ufds[fd]].Get_pk().c_str(), 16);
        mpz_pow_ui(npow2, npow2, 2);
        for(auto& x:Users[ufds[fd]].Encipher_data)
        {
            // cout << "x.second:\n" << x.second << endl;
            mpz_set_str(a, x.second.c_str(), 16);
            mpz_mul(rel, rel, a);
            mpz_mod(rel, rel, npow2);
        }

        char s_rel[KEYSIZE / 2 + 10];
        mpz_get_str(s_rel, 16, rel);
        // printf("s_rel:\n%s\n", s_rel);
        char* send_data = encode_msg(3, s_rel, s_rel);
        // cout << "strlen(send_data) " << strlen(send_data) << endl;
        secure_send(fd, send_data);
        printf("发送密文和成功\n");
        free(send_data);

    }

    return;
}


void encipher_file(char *path, vector<char*> &v, KEY &key)
{
    mpz_t enci_data;
    mpz_init(enci_data);
    char BUF[4096];
    FILE *f = fopen(path, "r");
    int index;
    unsigned long data;
    char str_index[40], str_endata[KEYSIZE / 2 + 10];
    while(fgets(BUF, sizeof(BUF), f) != NULL)
    {
        sscanf(BUF, "%d%lu", &index, &data);
        encipher_G(enci_data, key.pk_n, key.pk_g, key.pk_hs, data);
        mpz_get_str(str_endata, 16, enci_data);
        snprintf(str_index,sizeof(str_index), "%d", index);
        unsigned long a = decipher_G(enci_data, key.pk_n, key.sk_lambda, key.sk_mu);
        // printf("a:%lu\n", a);
        char *s = encode_msg(2, str_index, str_endata);
        // cout << s << endl;
        v.push_back(s);
    }
}