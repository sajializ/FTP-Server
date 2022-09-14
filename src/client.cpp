#include <fstream>
#include <iostream>
#include <system_error>

#include <unistd.h>
#include <arpa/inet.h> 
#include <sys/socket.h>

#include "lib/json/json/json.h"

#include "include/client.h"

using namespace std;

constexpr char Client::DEFAULT_SERVER_IP[];

Client::Client(const std::string& config_path)
{
    read_config_file(config_path);
    create_socket(socket_descriptor);
    create_socket(data_socket_descriptor);
    connect_socket(socket_descriptor, command_channel_port);
    connect_socket(data_socket_descriptor, data_channel_port);
}

Client::~Client()
{
    close(socket_descriptor);
    close(data_socket_descriptor);
}

string Client::send_request(const string& command)
{
    if (send(socket_descriptor, command.c_str(), command.size(), 0) < 0)
        throw system_error(errno, system_category(), "Sending data failed");
        
    if (send(data_socket_descriptor, command.c_str(), command.size(), 0) < 0)
        throw system_error(errno, system_category(), "Sending data failed");

    return receive_response(command);
}


void Client::create_socket(int& sock)
{
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        throw system_error(errno, system_category(), "Creating socket failed");
}

void Client::connect_socket(int sock, int port) const
{
    struct sockaddr_in server_address; 
    
    inet_pton(AF_INET, DEFAULT_SERVER_IP, &server_address.sin_addr);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port); 
   
    if (connect(sock, reinterpret_cast<struct sockaddr*>(&server_address), sizeof(server_address)) < 0) 
        throw system_error(errno, system_category(), "Connection failed");
}

string Client::receive_response(const string& command) const
{
    int size;
    uint8_t buffer[DEFAULT_BUFFER_SIZE];
    string command_type, command_arg;
    istringstream ss(command); 
    ss >> command_type;
    if (command_type == "ls")
    {
        memset(buffer, 0, DEFAULT_BUFFER_SIZE);
        if ((size = recv(data_socket_descriptor, buffer, DEFAULT_BUFFER_SIZE, 0)) < 0)
            throw system_error(errno, system_category(), "Receiving response failed");
        cout << buffer;
    }
    if (command_type == "retr")
    {
        ss >> command_arg;
        string file_name;
        if (command_arg.find('/') != string::npos)
            file_name = command_arg.substr(command_arg.rfind("/") + 1, command_arg.size());
        else
            file_name = command_arg;

        char file_size[DEFAULT_BUFFER_SIZE];
        if ((size = recv(data_socket_descriptor, file_size, DEFAULT_BUFFER_SIZE, 0)) < 0)
            throw system_error(errno, system_category(), "Receiving response failed");

        int length = atoi(file_size);
        if (length > -1)
        {
            fstream file(file_name, fstream::out | fstream::trunc);
            memset(buffer, 0, DEFAULT_BUFFER_SIZE);
            while (length > 0 && ((size = recv(data_socket_descriptor, buffer, DEFAULT_BUFFER_SIZE, 0)) > -1))
            {
                file.write(reinterpret_cast<char*>(buffer), size);
                memset(buffer, 0, DEFAULT_BUFFER_SIZE);
                length -= size;
            }
            file.close();
        }
    }

    memset(buffer, 0, DEFAULT_BUFFER_SIZE);
    if ((size = recv(socket_descriptor, buffer, DEFAULT_BUFFER_SIZE, 0)) < 0)
        throw system_error(errno, system_category(), "Receiving response failed");

    string result(reinterpret_cast<char*>(buffer));
    result.resize(size);
    return result;
}

void Client::read_config_file(const std::string& config_path)
{
    Json::CharReaderBuilder reader;
    Json::Value config_root;
    ifstream config_file(config_path);
    config_file >> config_root;

    command_channel_port = config_root["commandChannelPort"].asInt();
    data_channel_port = config_root["dataChannelPort"].asInt();
}