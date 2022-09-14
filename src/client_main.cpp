#include <iostream>

#include "include/client.h"
#include "include/responses.h"

using namespace std;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        cerr << "Too few args" << endl;
        return 1;
    }
    Client* client = new Client(argv[1]);
    while (true)
    {
        string command, response;
        getline(cin, command);
        response = client->send_request(command);
        cout << response << endl;
        if (response == QUIT_SUCCESS)
            break;
    }

    return 0;
}
