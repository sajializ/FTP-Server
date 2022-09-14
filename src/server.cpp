#include <system_error>

#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>

#include "lib/json/json/json.h"

#include "include/server.h"
#include "include/responses.h"
#include "include/service_thread.h"

using namespace std;

char Server::server_files_path[];

vector<User*> Server::users;
vector<string> Server::private_files;

Logger Server::logger;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

Server::Server(const string& config_path, const string& server_path)
{
    read_config_file(config_path);
    logger("Reading from config file successful");
    create_socket(command_socket_descriptor);
    create_socket(data_socket_descriptor);
    logger("Creating sockets successful");
    bind_socket(command_socket_descriptor, command_channel_port, &command_address);
    bind_socket(data_socket_descriptor, data_channel_port, &data_address);
    logger("Binding sockets successful");
    make_listen(command_socket_descriptor);
    make_listen(data_socket_descriptor);
    logger("Listening sockets successful");
    chdir(server_path.c_str());     // Change program folder.
    getcwd(server_files_path, DEFAULT_BUFFER_SIZE);
    logger("Server is up");
}

Server::~Server()
{
    close(command_socket_descriptor);
    close(data_socket_descriptor);
    for (vector<User*>::iterator user = users.begin(); user != users.end(); user++)
        delete *user;
    logger("Server shuts down");
}

pair<int, int> Server::accept_connection()
{
    int new_command_socket, new_data_socket;
    socklen_t command_address_length = sizeof(command_address);
    socklen_t data_address_length = sizeof(data_address);
    if ((new_command_socket = accept(command_socket_descriptor, reinterpret_cast<struct sockaddr*>(&command_address),
            &command_address_length)) < 0) 
        throw system_error(errno, system_category(), "Accept failed");
    if ((new_data_socket = accept(data_socket_descriptor, reinterpret_cast<struct sockaddr*>(&data_address),
            &data_address_length)) < 0) 
        throw system_error(errno, system_category(), "Accept failed");
    logger("Accepting new client successful");
    return pair<int, int>(new_command_socket, new_data_socket);
}

void* Server::manage_request(void* new_sockets)
{
    pair<int,int> sockets = *(static_cast<pair<int, int>*>(new_sockets));
    bool finish = false;
    User* logged_in_user;
    string username = "", log;
    ServiceThread service_thread(sockets);
    while (!finish)
    {
        int size;
        char buffer[DEFAULT_BUFFER_SIZE];
        memset(buffer, 0, DEFAULT_BUFFER_SIZE);
        if ((size = recv(sockets.first, buffer, DEFAULT_BUFFER_SIZE, 0)) < 0)
            throw system_error(errno, system_category(), "Receive command failed");
        else if (size == 0)
            break;

        vector<string> command = parse_command(buffer);
        string response = "";

        if (command[0] == "user")
        {
            if (verify_username(command[1], response))
                username = command[1];
        }
        else if (command[0] == "pass")
        {
            logged_in_user = verify_password(username, command[1], response);
            if (logged_in_user != nullptr)
            {
                service_thread.set_logged_in_user(logged_in_user);
                pthread_mutex_lock(&lock);
                logger(username + " has logged in successfully");
                pthread_mutex_unlock(&lock);
            }
        }
        else
        {
            log = "";
            response = service_thread.manage_command(command, finish, log);
            pthread_mutex_lock(&lock);
            logger(log);
            pthread_mutex_unlock(&lock);
        }

        send_buffer(sockets.first, response, response.size());
    }

    close(sockets.first);
    close(sockets.second);
    return nullptr;
}

size_t Server::send_buffer(int sock, const string& buffer, size_t n)
{
    int size;
    if ((size = send(sock, buffer.c_str(), n, 0)) < 0)
        throw system_error(errno, system_category(), "Send response failed");
    return size;
}

size_t Server::send_buffer(int sock, const string& buffer)
{
    int size;
    if ((size = send(sock, buffer.c_str(), DEFAULT_BUFFER_SIZE, 0)) < 0)
        throw system_error(errno, system_category(), "Send response failed");
    return size;
}

User* Server::verify_password(const string& username, const string& password, string& response) noexcept
{
    if (username == "")
    {
        response = BAD_SQ;
        return nullptr;
    }

    User* user;
    if ((user = find_user(username)) != nullptr && user->password == password)
    {
        response = PASSWORD_OK;
        return user;
    }
    response = INVALID_PASSWORD;
    return nullptr;
}

bool Server::verify_username(const string& username, string& response) noexcept
{
    if (find_user(username) != nullptr)
    {
        response = USERNAME_OK;
        return true;
    }

    response = INVALID_USERNAME;
    return false;
}

User* Server::find_user(const string& username) noexcept
{
    for (User* user : users)
        if (user->username == username)
            return user;
    return nullptr;
}

vector<string> Server::parse_command(char command[]) noexcept
{
    int index = 0;
    string word;
    istringstream ss(command); 
    vector<string> result;
    while (ss >> word)
        result.push_back(word);

    return result;
}

void Server::make_listen(int sock) const
{
    if (listen(sock, DEFAULT_LISTEN_QUEUE_SIZE) < 0) 
        throw system_error(errno, system_category(), "Listening socket failed");
}

void Server::bind_socket(int sock, int port, struct sockaddr_in* address) const
{
    address->sin_family = AF_INET; 
    address->sin_addr.s_addr = INADDR_ANY; 
    address->sin_port = htons(port); 
       
    if (bind(sock, reinterpret_cast<struct sockaddr*>(address), sizeof(*address)) < 0)
        throw system_error(errno, system_category(), "Binding socket failed");
}

void Server::create_socket(int& sock)
{
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        throw system_error(errno, system_category(), "Creating socket failed");

    int on = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &on, sizeof(on))) 
        throw system_error(errno, system_category(), "Set socket option failed");
}

void Server::read_config_file(const string& config_path)
{
    Json::CharReaderBuilder reader;
    Json::Value config_root;
    ifstream config_file(config_path);
    config_file >> config_root;

    command_channel_port = config_root["commandChannelPort"].asInt();
    data_channel_port = config_root["dataChannelPort"].asInt();

    for (Json::ValueIterator user = config_root["users"].begin(); user != config_root["users"].end(); user++)
        users.push_back(new User((*user)["user"].asString(), (*user)["password"].asString(),
                (*user)["admin"].asString() == "true", stoi((*user)["size"].asCString())));

    for (Json::ValueIterator user = config_root["files"].begin(); user != config_root["files"].end(); user++)
        private_files.push_back((*user).asString());
}

void Server::run()
{
    pthread_t thread;
    while (true)
    {
        pair<int, int> sockets = this->accept_connection();
        pthread_create(&thread, NULL, Server::manage_request, static_cast<void*>(&sockets.first));
    }
}