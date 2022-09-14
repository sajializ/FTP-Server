#include <memory>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <system_error>
#include <bits/stdc++.h> 

#include <ftw.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#include "include/service_thread.h"
#include "include/responses.h"
#include "include/server.h"

using namespace std;

ServiceThread::ServiceThread(pair<int, int> sockets)
{
    logged_in_user = nullptr;
    command_socket = sockets.first;
    data_socket = sockets.second;
}

void ServiceThread::set_logged_in_user(User* user) noexcept
{
    logged_in_user = user;
}
    
std::string ServiceThread::manage_command(const vector<string>& command, bool& finish, string& log)
{
    if (logged_in_user == nullptr)
    {
        if (command[0] == "ls" || command[0] == "retr")
            Server::send_buffer(data_socket, "");
        return NEED_LOGIN;
    }
    if (command[0] == "pwd")
    {
        if (command.size() != 1)
            return SYNTAX_ERROR;
        return pwd();
    }
    if (command[0] == "mkd")
    {
        if (command.size() != 2)
            return SYNTAX_ERROR;   
        if (mkdir(get_path(command[1]).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0)
            return ERROR;
        log = logged_in_user->username + " created " + command[1];
        return MKDIR_SUCCESS(command[1]);
    }
    if (command[0] == "dele")
    {
        if (command.size() != 3)
            return SYNTAX_ERROR;
        string result = dele(command);
        if (result != ERROR && result != SYNTAX_ERROR)
            log = logged_in_user->username + " removed " + command[2];
        return result;
    }
    if (command[0] == "cwd")
    {
        if (command.size() > 2)
            return SYNTAX_ERROR;
        if (command.size() == 1)
            return cwd("");
        return cwd(command[1]);
    }
    if (command[0] == "rename")
    {
        if (command.size() != 3)
            return SYNTAX_ERROR;
        if (!check_user_permission(command[1]))
            return FILE_UNAVAILABLE;
        if (rename(get_path(command[1]).c_str(), get_path(command[2]).c_str()) < 0)
            return ERROR;
        log = logged_in_user->username + " renamed " + command[1] + " to " + command[2];
        return RENAME_SUCCESS; 
    }
    if (command[0] == "quit")
    {
        if (command.size() != 1)
            return SYNTAX_ERROR;
        finish = true;
        log = logged_in_user->username + " left";
        return QUIT_SUCCESS;
    }
    if (command[0] == "ls")
    {
        if (command.size() != 1)
        {
            Server::send_buffer(data_socket, "");
            return SYNTAX_ERROR;
        }
        return ls(); 
    }
    if (command[0] == "retr")
    {
        if (command.size() != 2)
        {
            Server::send_buffer(data_socket, to_string(-1));
            return SYNTAX_ERROR;
        }
        string result = retr(command);
        if (result == RETR_SUCCESS)
            log = logged_in_user->username + " successfully downloaded " + command[1];
        return result;
    }
    if (command[0] == "help")
    {
        if (command.size() != 1)
            return SYNTAX_ERROR;
        return string("214\n")
                + string("USER [name], Its argument is used to specify the user’s string. It is used for user authentication.\n")
                + string("PASS [password], Its argument is used to match the user specified in the USER command.\n")
                + string("PWD, It is used to get the current directory. The response is the working directory path.\n")
                + string("MKD [name], Its argument is used to specify in which directory path, the new directory is to be created.\n")
                + string("DELE, Its argument is used to determine the name of the target filename or directory path that is to be rermoved.\n")
                + string("\tUse option -f [filename] to delete a specific file.\n")
                + string("\tUse option -d [directory path] to delete a specific directory.\n")
                + string("LS, displays all the file names in the current working directory.\n")
                + string("CWD [path], Changes the working directory and its argument is used to specify the target path to which the user wishes to switch.\n")
                + string("RENAME [from] [to], Its first argument is used to state the name of the file that is to be changed and its second argument is used as the new file name.\n")
                + string("RETR [name], Its argument is used to retrieve the named file from server and the file in question is downloaded upon completing the transfer.\n")
                + string("HELP, Enlists all the available commands along with a concise desciption.\n")
                + string("QUIT, logs out the current logged in user from the system.");
    }
    return ERROR;
}

bool ServiceThread::check_user_permission(const string& path) const noexcept
{
    string file_name = path.substr(path.rfind("/") + 1, path.size());
    if (find(Server::private_files.begin(), Server::private_files.end(), file_name) != Server::private_files.end() && !logged_in_user->is_admin)
        return false;
    return true;
}

string ServiceThread::get_path(const string& name) const noexcept
{
    string result = "";
    for (auto wd : working_directory)
        result += wd + "/";
    return result + name;
}

string ServiceThread::retr(const vector<string>& command) const noexcept
{
    uint8_t block[Server::DEFAULT_BUFFER_SIZE];
    int file_size = get_file_size(command[1]);
    if (file_size < 0)
    {
        Server::send_buffer(data_socket, to_string(file_size));
        return ERROR;
    }
    if (!check_user_permission(command[1]))
    {
        Server::send_buffer(data_socket, to_string(-1));
        return FILE_UNAVAILABLE;
    }
    if (logged_in_user->data_cap < file_size)
    {
        Server::send_buffer(data_socket, to_string(-1));
        return DATA_CAP_ERROR;
    }
    logged_in_user->data_cap -= file_size;
    Server::send_buffer(data_socket, to_string(file_size));
    FILE* file = fopen(get_path(command[1]).c_str(), "r");
    while (!feof(file))
    {
        int size = fread(block, sizeof(uint8_t), Server::DEFAULT_BUFFER_SIZE, file);
        send(data_socket, block, size, 0);
    }
    fclose(file);
    return RETR_SUCCESS;
}

string ServiceThread::ls() const noexcept
{
    dirent* dir_path;
    DIR* path = opendir(get_path(".").c_str());
    string result;
    while ((dir_path = readdir(path)) != nullptr)
    {
        string name = string(dir_path->d_name);
        if (name != "." && name != "..")
            result += name + '\n';
    }
    Server::send_buffer(data_socket, result.c_str(), result.size() == 0 ? Server::DEFAULT_BUFFER_SIZE : result.size());
    return LS_SUCCESS; 
}

string ServiceThread::pwd() const noexcept
{
    return PWD_SUCCESS("/" + get_path(""));
}

string ServiceThread::cwd(const string& path) noexcept
{
    if (path == "")
    {
        working_directory.erase(working_directory.begin(), working_directory.end());
        return CWD_SUCCESS;
    }
    if (path[0] == '/')
    {
        cwd("");
        return cwd(path.substr(1, path.size()));
    }
    if (path == "..")
    {
        if (working_directory.size() != 0)
            working_directory.pop_back();
        return CWD_SUCCESS;
    }
    string dir;
    istringstream ss(path);
    struct stat sb;
    if (stat(get_path(path).c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
    {
        while (getline(ss, dir, '/'))
            working_directory.push_back(dir);
        return CWD_SUCCESS;
    }
    return ERROR;
}

int ServiceThread::get_file_size(const string& filename) const noexcept
{
    struct stat stat_buf;
    int rc = stat(get_path(filename).c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

string ServiceThread::dele(const vector<string>& command) const noexcept
{
    if (!check_user_permission(command[2]))
        return FILE_UNAVAILABLE;
    int result = -1;
    if (command[1] == "-d")
        result = nftw(get_path(command[2]).c_str(), ServiceThread::del, 64, FTW_DEPTH | FTW_PHYS);
    else if (command[1] == "-f")
        result = unlink(get_path(command[2]).c_str());
    else
        return SYNTAX_ERROR;
    if (result < 0)
        return ERROR;
    return DELE_SUCCESS(command[2]);
}

int ServiceThread::del(const char* path, const struct stat* sb, int typeflag, struct FTW* ftwbuf)
{
    return S_ISREG(sb->st_mode) ? -1 : remove(path);
}
