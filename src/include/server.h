#ifndef SERVER_H_
#define SERVER_H_

#include <vector>
#include <string>

#include <arpa/inet.h> 

#include "user.h"
#include "logger-inl.h"
#include "service_thread.h"

class Server
{
public:
    Server(const std::string& config_path, const std::string& server_path);
    ~Server();
    void run();

private:
    void create_socket(int& sock);
    void make_listen(int sock) const;
    std::pair<int, int> accept_connection();
    void read_config_file(const std::string& config_path);
    void bind_socket(int sock, int port, struct sockaddr_in* address) const;

    static void* manage_request(void* new_sockets);
    static User* find_user(const std::string& username) noexcept;
    static size_t send_buffer(int sock, const std::string& buffer);
    static std::vector<std::string> parse_command(char command[]) noexcept;
    static size_t send_buffer(int sock, const std::string& buffer, size_t n);
    static bool verify_username(const std::string& username, std::string& response) noexcept;
    static User* verify_password(const std::string& logged_in_user, const std::string& password, std::string& response) noexcept;

    static constexpr size_t DEFAULT_LISTEN_QUEUE_SIZE = 5;
    static constexpr size_t DEFAULT_BUFFER_SIZE = 4096;
    
    static char server_files_path[DEFAULT_BUFFER_SIZE];
    static std::vector<std::string> private_files;
    static std::vector<User*> users;
    static Logger logger;

    struct sockaddr_in command_address;
    struct sockaddr_in data_address;

    std::string config_path;
    int command_socket_descriptor;
    int data_socket_descriptor;
    int command_channel_port;
    int data_channel_port;

    friend class ServiceThread;
};

#endif