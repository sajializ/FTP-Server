#ifndef USER_H_
#define USER_H_

#include <vector>
#include <string>

class User
{
public:
    std::string username;
    std::string password;
    bool is_admin;
    size_t data_cap;
    
    User(std::string _username, std::string _password, bool _is_admin, size_t _data_cap) :
        username(_username),
        password(_password),
        is_admin(_is_admin),
        data_cap(_data_cap) 
    {
    }
};

#endif