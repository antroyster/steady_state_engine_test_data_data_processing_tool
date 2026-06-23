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
std::string naCheckStr(const T& value) {
    if constexpr (std::is_floating_point_v<T>) {
        if (std::isnan(value)) return "N/A";
    }
    else if constexpr (std::is_integral_v<T>) {
        if (value == -1) return "N/A";
    }
    return std::to_string(value);
};
template<typename T>
bool naCheckBool(const T& value) {
    if constexpr (std::is_floating_point_v<T>) {
        if (std::isnan(value)) return true;
    }
    else if constexpr (std::is_integral_v<T>) {
        if (value == -1) return true;
    }
    return false;
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
        int logsafterTrigger =-1; // number of logs to capture after trigger
    };
    struct DataInclusion {
        std::vector<int> triggerPositive;
        std::vector<int> triggerNegative;
        int id;
        std::vector<int> rowInclusion;
    };
    struct StabilityCheck {
        size_t activation;
        size_t stabcheckATrig;
        bool trigger;
    };
    void stabilityCheck(dataAnalysis::StabilityCheck& stabilitycheck_i, dataAnalysis::DataInclusion& result, size_t& i, size_t& logsBTrig);
    void generatestabilityVector(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements);
    void dataPrune(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements);
	void askstabilityRequirements(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements);
	std::vector<int> stabcPruning(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements, size_t& numofRows, int id);
    std::vector<int> minmaxPruning(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements, size_t& numofRows, int id, bool minmax);
    size_t numofRows(const std::vector<dataLogger::Channel>& channels);
    void triggerFlip(dataAnalysis::DataInclusion& result, size_t& numofRows, bool triggerfliptype);
    std::vector<int>logicalandComparison(std::vector<std::vector<int>>& comparitor, size_t numofRows);
    std::vector<StabilityRequirement> stabilityRequirements;
	std::vector<int> rowInclusion;
private:

};
#endif // DATAANALYSIS_H
