
#include "dataLogger.h"
//#include "dataAnalysis.h"
//#include "dataDisplay.h"
#include <iostream> 
#include <fstream>
#include <stdexcept> 
#include <filesystem>
#include <vector>
#include <utility>
#include <string>

int main()
{
    try {
       dataLogger dataLogger_i;   // create object
       dataLogger_i.readStore();
    }
    catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
    }

   // std::pair<double,double>& criteriaRef = dataAnalysis.inputCriteria();
    //std::vector<double>& results = dataAnalysis.pruning(std::pair<int,int>& criteriaRef);
    //dataLogger.write();
    //dataDisplay.terminalReport(results);
    return 0;
}