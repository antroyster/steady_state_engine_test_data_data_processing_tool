// steady_state_engine_test_data_processing_tool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
// steady_state_engine_test_data_processing_tool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "dataLogger.h"
#include "dataAnalysis.h"
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

    return 0;
}
