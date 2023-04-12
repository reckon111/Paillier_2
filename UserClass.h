#pragma once

#include <string>
#include <unordered_map>

using namespace std;

class User
{
private:
    string m_name;
    string m_passwd;
    int m_clifd;
    string m_pk_n;
public:
    unordered_map<string, string> Encipher_data;
    User();
    User(int clifd, string name, string passwd);
    void StoreEncidata(char* data1, char* data2);
    void Record_pk(char* pk_n);
    string GetPasswd();
    string Get_pk();
    ~User();
};


