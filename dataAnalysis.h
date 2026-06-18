#pragma once

#ifndef DATAANALYSIS_H
#define DATAANALYSIS_H

#include <iostream> 
#include <fstream>
#include <stdexcept> 
#include <filesystem>
#include <vector>
#include <utility>
#include <limits>
#include <string>

template<typename T>
std::string naCheck(const T& value) {
    if constexpr (std::is_floating_point_v<T>) {
        if (std::isnan(value)) return "N/A";
    }
    else if constexpr (std::is_integral_v<T>) {
        if (value == -1) return "N/A";
    }
    return std::to_string(value);
};

class dataAnalysis {
public:
    dataAnalysis() = default;
    struct StabilityRequirement {
        int stabilitywaitC =-1; // in number of logs
        int stabilitywaitROC = -1; // in number of logs
		double stabilitychangeTrigger = std::numeric_limits<double>::quiet_NaN(); // 
        double stabilitychangeRange = std::numeric_limits<double>::quiet_NaN(); // 
        double stabilityrocTrigger = std::numeric_limits<double>::quiet_NaN(); // in value/logs 
        double min = std::numeric_limits<double>::quiet_NaN(); // min value
        double max = std::numeric_limits<double>::quiet_NaN(); // max value 
        int logsbeforeTrigger =-1; // number of logs to capture before trigger 
        int logsafterEnd =-1; // number of logs to capture after trigger
    };
    void generatestabilityVector(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements);
	void askstabilityRequirements(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements);
    std::vector<StabilityRequirement> stabilityRequirements;
private:

};
#endif // DATAANALYSIS_H
