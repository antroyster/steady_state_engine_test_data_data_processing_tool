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
		dataAnalysis dataAnalysis_i; // create object
        dataLogger_i.readStore();
		dataAnalysis_i.initializeStabilityRequirements(dataLogger_i.getchannelsData(), dataAnalysis_i.stabilityRequirements);
        dataAnalysis_i.promptStabilityRequirements(dataLogger_i.getchannelsData(), dataAnalysis_i.stabilityRequirements);
        dataAnalysis_i.dataPrune(dataLogger_i.getchannelsData(), dataAnalysis_i.stabilityRequirements);
        dataLogger_i.storeWrite(dataLogger_i.getchannelsData(), dataLogger_i.getmetadata(), dataAnalysis_i.rowInclusion, dataAnalysis_i.numofRows(dataLogger_i.getchannelsData()));
        
    }
    catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
    }

    return 0;
}
