#include "UserClass.h"

User::User()
{
}
User::User(int clifd, string name, string passwd)
{
    m_clifd = clifd;
    m_name = name;
    m_passwd = passwd;
}
string User::GetPasswd()
{
    return m_passwd;
}
void User::Record_pk(char* pk_n)
{
    m_pk_n = string(pk_n);
}
string User::Get_pk()
{
    return m_pk_n;
}

void User::StoreEncidata(char* data1, char* data2)
{
    Encipher_data[string(data1)] = string(data2);
}

User::~User()
{
}