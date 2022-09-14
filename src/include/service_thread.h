#ifndef SERVICE_THREAD_H_
#define SERVICE_THREAD_H_

#include <vector>
#include <string>

#include "user.h"

class ServiceThread
{
public:
    ServiceThread(std::pair<int, int> sockets);
    void set_logged_in_user(User* user) noexcept;
    std::string manage_command(const std::vector<std::string>& command, bool& finish, std::string& log);

private:
    int get_file_size(const std::string& filename) const noexcept;
    bool check_user_permission(const std::string& file_name) const noexcept;
    std::string ls() const noexcept;
    std::string pwd() const noexcept;
    std::string cwd(const std::string& path) noexcept;
    std::string get_path(const std::string& name) const noexcept;
    std::string retr(const std::vector<std::string>& command) const noexcept;
    std::string dele(const std::vector<std::string>& command) const noexcept;

    static int del(const char* path, const struct stat* sb, int typeflag, struct FTW* ftwbuf);

    User* logged_in_user;
    std::vector<std::string> working_directory;
    int command_socket;
    int data_socket;
};

#endif