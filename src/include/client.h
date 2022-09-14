#ifndef CLIENT_H_
#define CLIENT_H_

#include <vector>
#include <string>

class Client
{
public:
    Client(const std::string& config_path);
    ~Client();

    std::string send_request(const std::string& command);

private:
    void create_socket(int& sock);
    void connect_socket(int sock, int port) const;
    std::string receive_response(const std::string& command) const;
    void read_config_file(const std::string& config_path);

    static constexpr size_t DEFAULT_BUFFER_SIZE = 4096;
    static constexpr char DEFAULT_SERVER_IP[] = "127.0.0.1";

    int socket_descriptor;
    int data_socket_descriptor;
    int command_channel_port;
    int data_channel_port;
};

#endif