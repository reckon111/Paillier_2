#ifndef __DATAMANAGE_H__
#define __DATAMANAGE_H__
#include <gmp.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <unordered_map>
#include "paillier.h"
#include "UserClass.h"
#include <vector>

using namespace std;

#define MYPORT  4000
#define BUFFER_SIZE 1024
#define KEYSIZE 4096
#define BASE 16
#define NAMESIZE 128
#define DATASIZE 2000
#define HEADER_SIZE 9          // 消息头长度
#define MSG1BYTES_LEN 4 
#define MSG2BYTES_LEN 4
#define TYPE_LEN 1


typedef struct{
    char* data1;
    char* data2;
    char* error;
    int type;
}RECVD_MSG;

typedef struct{
    char* data1;
    char* data2;
}ENCIP_MSG;


class KEY
{
public:
    mpz_t pk_n, pk_g, pk_hs, sk_lambda, sk_mu;
public:
    KEY(){
        mpz_init(pk_n);
        mpz_init(pk_g);
        mpz_init(pk_hs);
        mpz_init(sk_lambda);
        mpz_init(sk_mu);
        key_generate_G(pk_n, pk_g, pk_hs, sk_lambda, sk_mu, KEYSIZE);
        printf("key generate\n");
    }
    ~KEY()
    {
        mpz_clear(pk_n);
        mpz_clear(pk_g);
        mpz_clear(pk_hs);
        mpz_clear(sk_lambda);
        mpz_clear(sk_mu);
    }
};


void secure_send(int fd, char* send_data);
void* Query(void* args);
int bits_10baseofnumber(int n);
void my_fgets(char *str, int size);
void secure_send(int fd, char* send_data);
int secure_recv(int sock_cli, RECVD_MSG *recvd_msg);
char* encode_msg(int type, char* msg1, char* msg2);
void rd2ed(struct rawdata *rdata, struct encidata *edata, int size, mpz_t pk_n, mpz_t pk_g);
void edata_init(struct encidata* edata, int size);
void edata_clear(struct encidata* edata, int size);
char* make_msg(char* fmt,mpz_t msg1);
// void shareget(Share_data* share_data);
void ServerDealMasseage(RECVD_MSG *recvd_msg, unordered_map<string, User> &Users, int fd, unordered_map<int, string> &ufds);

void encipher_file(char *path, vector<char*> &v, KEY &key);
#endif
