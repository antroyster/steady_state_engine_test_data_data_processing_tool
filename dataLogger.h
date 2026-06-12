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
    private:
        std::pair<double,double> criteria;
        std::vector<std::string> metadata;     
        std::vector<double> inputData; 
};
#endif 