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
    void storeWrite(const std::vector<Channel>& channels, const std::vector<std::string>& metadata, std::vector<int>& rowInclusion,size_t numofRows);
    void getMetadata(std::ifstream& insertedFile);
    void getChannels(std::ifstream& insertedFile);
    void getData(std::ifstream& insertedFile);
    const std::vector<Channel>& getchannelsData() const;
    const std::vector<std::string>& getmetadata() const; 
private:
    std::vector<std::string> metadata;
    std::vector<Channel> channels;
    std::string fileName;
    
};

#endif // DATALOGGER_H

