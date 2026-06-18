#include "dataLogger.h"
#include "dataAnalysis.h"
#include <iostream> 
#include <fstream>
#include <stdexcept> 
#include <filesystem>
#include <vector>
#include <utility>
#include <string>
#include <sstream>
#include <limits>
#include <iomanip>
#define DEBUG

void dataAnalysis::generatestabilityVector(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements) {
#ifdef DEBUG
    std::cout << "debug:Generate Stability Vector Called " << std::endl;
#endif // DEBUG

    for (const auto& element : channels) {
        stabilityRequirements.push_back(StabilityRequirement{});

    }
    #ifdef DEBUG
        std::cout << "debug: stabilityRequirements size is " << stabilityRequirements.size() << std::endl;
    #endif // DEBUG
}

void dataAnalysis::askstabilityRequirements(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements) {
    int tablewidth = 12;
    std::string string;
    std::string id_str;
    int id_int;
    bool exitflag = false;
    auto truncate = [tablewidth](const std::string& input) {
        return input.size() > tablewidth ? input.substr(0, tablewidth - 1) : input;
        };
    while (!exitflag) {
        std::cout << std::left
            << std::setw(tablewidth) << "ID"
            << std::setw(tablewidth) << "Channel"
            << std::setw(tablewidth) << "Units"
            << std::setw(tablewidth) << "StabCTrig"
            << std::setw(tablewidth) << "StabCRange"
            << std::setw(tablewidth) << "WaitATrigC"
            << std::setw(tablewidth) << "StabROCTrig"
            << std::setw(tablewidth) << "WaitAROC"
            << std::setw(tablewidth) << "Min"
            << std::setw(tablewidth) << "Max"
            << std::setw(tablewidth) << "LogsBTrig"
            << std::setw(tablewidth) << "LogsATrig"
            << "\n";
        std::cout << std::string(tablewidth * 12, '-') << "\n";
        for (const auto& element : channels) {
            std::cout << std::left
                << std::setw(tablewidth) << element.index
                << std::setw(tablewidth) << truncate(element.id)
                << std::setw(tablewidth) << truncate(element.units)
                << std::setw(tablewidth) << naCheck(stabilityRequirements[element.index].stabilitychangeTrigger)
                << std::setw(tablewidth) << naCheck(stabilityRequirements[element.index].stabilitychangeRange)
                << std::setw(tablewidth) << naCheck(stabilityRequirements[element.index].stabilitywaitC)
                << std::setw(tablewidth) << naCheck(stabilityRequirements[element.index].stabilityrocTrigger)
                << std::setw(tablewidth) << naCheck(stabilityRequirements[element.index].stabilitywaitROC)
                << std::setw(tablewidth) << naCheck(stabilityRequirements[element.index].min)
                << std::setw(tablewidth) << naCheck(stabilityRequirements[element.index].max)
                << std::setw(tablewidth) << naCheck(stabilityRequirements[element.index].logsbeforeTrigger)
                << std::setw(tablewidth) << naCheck(stabilityRequirements[element.index].logsafterEnd)
                << "\n";
        }
        std::cout << "Please Add Numerical Stability Requirements in the format \"ID:StabCTrig:StabCRange:StabROCTrig:StabROCRange:WaitATrig:Min:Max:LogsBTrig:LogsATrig\"" << std::endl;
        std::cout << "For Information on the required input format type I , to exit type E" << std::endl;
        std::cin >> string;
		if (string == "I" || string == "i") {
            std::string leave;
            std::cout << "Input format: ID:StabCTrig:StabCRange:WaitATrigC:StabROCTrig:WaitATrigROC:Min:Max:LogsBTrig:LogsATrig\n";
            std::cout << "To leave a parameter unset, leave it blank: e.g. 2:1800::30:-15:50:::10:10\n";
            std::cout << "\n";

            std::cout << "--- REFERENCES ---\n";
            std::cout << "\n";

            std::cout << "ID\n";
            std::cout << "  The index of the channel you wish to change, as shown in the table above.\n";
            std::cout << "  e.g. 7\n";
            std::cout << "\n";

            std::cout << "StabCTrig  (Stability Change Trigger)\n";
            std::cout << "  A threshold value. When the channel data crosses this value, data capture begins.\n";
            std::cout << "  Capture ends when the data crosses back over the threshold in the opposite direction,\n";
            std::cout << "  or when WaitATrigC expires.\n";
            std::cout << "  If no StabCTrig is set, the reference point defaults to the first value in the dataset\n";
            std::cout << "  or the first value following the end of the previous capture window.\n";
            std::cout << "  e.g. 1800 (rpm) — capture begins when speed drops below 1800 rpm\n";
            std::cout << "\n";

            std::cout << "StabCRange  (Stability Change Range)\n";
            std::cout << "  Optionally applied around StabCTrig. Once capture has started, the channel\n";
            std::cout << "  must leave this range before capture can end. This prevents capture ending prematurely\n";
            std::cout << "  due to small oscillations around the threshold.\n";
            std::cout << "  e.g. 10 (rpm)  once value is set at +-  \"10\", capture will start/end when it enters/leaves the \n";
            std::cout << "  1790\"/\"1810 , or utill WaitATrigC expires.\n";
            std::cout << "\n";

            std::cout << "WaitATrigC  (Wait After Trigger - Change)\n";
            std::cout << "  Overruling wait applied to StabCTrig captures only. Once a StabCTrig capture begins,\n";
            std::cout << "  capture will end after this many logs regardless of whether the threshold has been\n";
            std::cout << "  recrossed. Any further StabCTrig activations within this window are ignored.\n";
            std::cout << "  e.g. 50  capture ends after 50 logs from the initial StabCTrig activation.\n";
            std::cout << "\n";

            std::cout << "StabROCTrig  (Stability Rate-of-Change Trigger)\n";
            std::cout << "  A rate-of-change threshold evaluated over a rolling 10-log window (first log in,\n";
            std::cout << "  first log out). When the rate of change between the first and last log in the window\n";
            std::cout << "  meets or exceeds this value, data capture begins. Capture ends when the rate of change\n";
            std::cout << "  in the window no longer meets the threshold, or when WaitATrigROC expires.\n";
            std::cout << "  The value is expressed in units per 10 logs, so for 1Hz data a rate of \"-150\" rpm\/s\\n";
            std::cout << "  corresponds to a StabROCTrig value of \"-150\"\n";
            std::cout << "  Both positive and negative rates of change are supported.\n";
            std::cout << "  Note: StabROCTrig activations are typically short-lived. It is strongly recommended\n";
            std::cout << "  to pair this with WaitATrigROC to capture a meaningful window of data.\n";
            std::cout << "  e.g. \"-150\"  triggers when engine speed falls by more than 150 rpm over 10 logs (1Hz data)\n";
            std::cout << "\n";

            std::cout << "WaitATrigROC  (Wait After Trigger - Rate of Change)\n";
            std::cout << "  Overruling wait applied to StabROCTrig captures only. Once a StabROCTrig capture\n";
            std::cout << "  begins, capture will end after this many logs regardless of the current rate of change.\n";
            std::cout << "  Any further StabROCTrig activations within this window are ignored.\n";
            std::cout << "  e.g. 50  capture ends after 50 logs from the initial StabROCTrig activation.\n";
            std::cout << "\n";

            std::cout << "Min / Max  (Inclusion Filter)\n";
            std::cout << "  Passive filters applied independently of all trigger categories. Any data row where\n";
            std::cout << "  the channel value falls outside the Min/Max range will be excluded from the output.\n";
            std::cout << "  Min and Max can be set independently of each other.\n";
            std::cout << "  e.g. Min 600, Max 800 (kPa)  only rows where the channel reads between 600\-\800 kPa\n";
            std::cout << "  will be retained.\n";
            std::cout << "\n";

            std::cout << "LogsBTrig  (Logs Before Trigger)\n";
            std::cout << "  The number of logs to include before the start of any capture window.\n";
            std::cout << "  e.g. 10  the 10 logs immediately preceding the trigger activation are included.\n";
            std::cout << "\n";

            std::cout << "LogsATrig  (Logs After Trigger End)\n";
            std::cout << "  The number of logs to include after the end of any capture window.\n";
            std::cout << "  e.g. 10  the 10 logs immediately following the capture end point are included.\n";
            std::cout << "\n";
            std::cout << " Type any Character + Energy Key to Continue" <<std::endl;
            std::cin >> leave;
             
			continue;
		}
		else if (string == "E" || string == "e"){
            exitflag = true;
			break;
		}
        else
        {
            try {
                std::stringstream sString(string);
                std::getline(sString, id_str, ':');
                id_int = std::stoi(id_str);
                int inputPosition = 1;
                while (std::getline(sString, string, ':')) {
                    if (string.empty() || string == "\n" ) {
                        inputPosition++;
                        continue;
                     }
                    switch (inputPosition) {
                    case 1:
                        stabilityRequirements[id_int].stabilitychangeTrigger = std::stod(string);
                        inputPosition++;
                        break;
                    case 2:
                        stabilityRequirements[id_int].stabilitychangeRange = std::stod(string);
                        inputPosition++;
                        break;
                    case 3:
                        stabilityRequirements[id_int].stabilitywaitC = std::stoi(string);
                        inputPosition++;
                        break;
                    case 4:
                        stabilityRequirements[id_int].stabilityrocTrigger = std::stod(string);
                        inputPosition++;
                        break;
                    case 5:
                        stabilityRequirements[id_int].stabilitywaitROC = std::stoi(string);
                        inputPosition++;
                        break;
                    case 6:
                        stabilityRequirements[id_int].min = std::stod(string);
                        inputPosition++;
                        break;

                    case 7:
                        stabilityRequirements[id_int].max = std::stod(string);
                        inputPosition++;
                        break;
                    case 8:
                        stabilityRequirements[id_int].logsbeforeTrigger = std::stoi(string);
                        inputPosition++;
                        break;
                    case 9:
                        stabilityRequirements[id_int].logsafterEnd = std::stoi(string);
                        inputPosition++;
                        break;
                    }
                }
			}
			catch (const std::exception& error) {
				std::cerr << "Error: " << error.what() << std::endl;
				std::cerr << "Please enter the stability requirements in the correct format." << std::endl;
			}
		}
	}
};






