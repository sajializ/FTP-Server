#ifndef LOGGER_H_
#define LOGGER_H_

#include <ctime>    
#include <chrono>
#include <string>
#include <fstream>

class Logger
{
public:
    Logger()
    {
        file.open("log.txt", std::fstream::out | std::fstream::app);
    }
    ~Logger()
    {
        file.close();
    }
    void log(const std::string& message)
    {
        if (message.size() != 0)
        {
            auto clock = std::chrono::system_clock::now();
            std::time_t time = std::chrono::system_clock::to_time_t(clock);
            file << message << " At " <<  std::ctime(&time) << std::endl;
        }
    }
    void operator()(std::string msg) 
    {
        log(msg);
    }

private:
    std::fstream file;
};

#endif