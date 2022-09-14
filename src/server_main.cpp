#include <iostream>

#include "include/server.h"

using namespace std;

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        cerr << "Too few args" << endl;
        return 1;
    }
    Server* server = new Server(argv[1], argv[2]);
    server->run();
    return 0;
}