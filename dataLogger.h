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

class dataLogger{
    public:
    dataLogger() = default;
    void readStore();
    void getMetadata(std::ifstream& insertedFile);
    void getChannels(std::ifstream& insertedFile);
    void getData(std::ifstream& insertedFile);
    private:
        std::pair<double,double> criteria;
        std::vector<std::string> metadata;     
        struct Channel {
            std::vector<double>data;
            std::string id;
            std::string units;
        };
        std::vector<Channel> channels;
};
#endif 