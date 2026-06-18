#pragma once

#ifndef DATALOGGER_H
#define DATALOGGER_H

#include <iostream> 
#include <fstream>
#include <stdexcept> 
#include <filesystem>
#include <vector>
#include <utility>
#include <string>

class dataLogger {
public:
    dataLogger() = default;
    struct Channel {
        std::vector<double>data;
        std::string id;
        std::string units;
        int index;
    };
    void readStore();
    void getMetadata(std::ifstream& insertedFile);
    void getChannels(std::ifstream& insertedFile);
    void getData(std::ifstream& insertedFile);
    const std::vector<Channel>& getchannelsData() const;
private:
    std::vector<std::string> metadata;
    std::vector<Channel> channels;
    
};

#endif // DATALOGGER_H

