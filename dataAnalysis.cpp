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
#include <algorithm>
#include <functional>
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
size_t dataAnalysis::numofRows(const std::vector<dataLogger::Channel>& channels) {
    auto largestnumofRows = std::max_element(
        channels.begin(),
        channels.end(),
        [](const dataLogger::Channel& a, const dataLogger::Channel& b)
        {return a.data.size() < b.data.size();}
        );
    return largestnumofRows->data.size();
}
void dataAnalysis::triggerFlip(dataAnalysis::DataInclusion& result, size_t& numofRows, bool triggerfliptype)
{
    bool negativetriggerrequired = 0;
    bool positvetriggerrequired = 0;
    for (size_t i = 0; i < numofRows; i++) {
        if (positvetriggerrequired == 0 && negativetriggerrequired == 0)
        {
            if (result.triggerPositive[i] == 1)
            {
                negativetriggerrequired = 1;
                result.rowInclusion[i] = !triggerfliptype;
            }
            //else if (result.triggerNegative[i] == 1)
            {
               // positvetriggerrequired = 1;
               // result.rowInclusion[i] = !triggerfliptype;
            }
        }
        //if (positvetriggerrequired == 1)
        {
            //result.rowInclusion[i] = !triggerfliptype;
            //if (result.triggerPositive[i] == 1)
            {
               // positvetriggerrequired = 0;

            }
        }
        if (negativetriggerrequired == 1)
        {
            result.rowInclusion[i] = !triggerfliptype;
            if (result.triggerNegative[i] == 1)
            {
                negativetriggerrequired = 0;
            }

        }

    }
}
void dataAnalysis::stabilityCheck(dataAnalysis::StabilityCheck& stabilityCheck_i, dataAnalysis::DataInclusion& result, size_t& i, size_t& logsBTrig)
{
#ifdef DEBUG
    std::cout << "Debug: stability check values" << "current : " << static_cast<int>(i) << " stabcheckATrig: " << static_cast<int>(stabilityCheck_i.stabcheckATrig) << " activation: " << static_cast<int>(stabilityCheck_i.activation) << " trigger: " << static_cast<int>(stabilityCheck_i.trigger) << std::endl;
#endif // DEBUG

    if (stabilityCheck_i.trigger && ( i >= (stabilityCheck_i.activation+stabilityCheck_i.stabcheckATrig)) )
    {
        #ifdef DEBUG
        std::cout << "debug:  has gone beyond stability threshold at " << i << std::endl;
        #endif // DEBUG

        for (size_t j = (i - 2); j > (i - 1 - stabilityCheck_i.stabcheckATrig); j--)
        {

            result.triggerPositive.at(j) = 0;
           
        }
        if (logsBTrig)
        {
            result.triggerPositive.at(i - logsBTrig) = 1;
        }
        else
        {
            result.triggerPositive.at(i) = 1;
        }
        //result.triggerNegative.at(i) = 1;
        stabilityCheck_i.trigger = false;
    }
}
std::vector<int> dataAnalysis::stabROCPruning(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements, size_t& numofRows, int id) {
    dataAnalysis::DataInclusion result;
    double criteria = stabilityRequirements[id].stabilityrocTrigger;
    size_t logsSample = stabilityRequirements[id].stabilitywaitROC;
    size_t logsATrig = stabilityRequirements[id].logsafterTrigger;
    size_t logsATrig_do = -1;
    size_t logsBTrig = stabilityRequirements[id].logsbeforeTrigger;
    bool positvenegative = true;
    bool signCriteria = (criteria >= 0) ? true : false;
    
    
    if (naCheckBool(logsATrig))
    {
        logsATrig = 0;
    }
    if (naCheckBool(logsBTrig))
    {
        logsBTrig = 0;
    }
    if (naCheckBool(logsSample))
    {
        logsSample = 0;
    }
    for (size_t i = 0; i < numofRows; i++)
    {
        result.id = id;
        result.rowInclusion.push_back(0);
        result.triggerNegative.push_back(0);
        result.triggerPositive.push_back(0);



    }
#ifdef DEBUG
    std::cout << "debug: generated result vector" << std::endl;
#endif // DEBUG
    
    
    for (size_t i = logsSample; i < (numofRows); i++)
    {
        try {
            auto current = channels[id].data[i];
            auto previous = channels[id].data[i - logsSample];
            auto rateofChange = (current - previous) / logsSample;
            bool signData = (rateofChange >= 0) ? true : false;
#ifdef DEBUG
            if (i > 1300 || i < 20)
            {
                std::cout << "debug: current value at " << i << " is " << current << " and previous value at " << previous << std::endl;
                std::cout << "debug: rate of change at " << i << " is " << rateofChange << " and criteria is " << criteria << std::endl;
                std::cout << "debug: signData is " << signData << " and signCriteria is " << signCriteria << std::endl;
                
            }
#endif // DEBUG

            if ((std::abs(rateofChange) >= std::abs(criteria)) && positvenegative == true)
            {

                
                if ((signCriteria != signData))
                {
                    continue;
                }
                positvenegative = false;
#ifdef DEBUG
                std::cout << "debug: positvenegative set to " << positvenegative << " at row " << i << std::endl;
#endif // DEBUG
                if (logsBTrig)
                {

                    result.triggerPositive.at(i+1 - logsBTrig) = 1;
                }
                else
                {
                    result.triggerPositive.at(i+1) = 1;
                }

                if (logsATrig)
                {
                    logsATrig_do = logsATrig + i + 1;
                }
                
            }
            else if (positvenegative == false)
            {
                if ((std::abs(rateofChange) <= std::abs(criteria)) && (signCriteria == signData))
                {

                    positvenegative = true;
#ifdef DEBUG
                    std::cout << "debug: positvenegative set to " << positvenegative << " at row " << i << std::endl;
#endif // DEBUG
                    result.triggerNegative.at(i) = 1;
                }
                else if (signCriteria != signData)
                {
                    positvenegative = true;
#ifdef DEBUG
                    std::cout << "debug: positvenegative set to " << positvenegative << " at row " << i << std::endl;
#endif // DEBUG
                    result.triggerNegative.at(i) = 1;
                }

            }
            if (logsATrig_do == i)
            {
                for (size_t j = (i - 2); j > (i - 1 - logsATrig); j--)
                {
                    result.triggerNegative.at(j) = 0;
                }
                result.triggerNegative.at(i - 1) = 1;
                positvenegative = true;
#ifdef DEBUG
                std::cout << "debug: positvenegative set to " << positvenegative << " at row " << i << std::endl;
#endif // DEBUG
                logsATrig_do = -1;
            }
		}
        catch (const std::out_of_range&) {
            std::cout << "debug: out of range exception at " << i << std::endl;
            break;
        }
    }
    if (logsATrig_do != -1 && !naCheckBool(logsATrig))
    {
        result.triggerNegative.at(numofRows - 1) = 1;
    }
    dataAnalysis::triggerFlip(result, numofRows, 0);
#ifdef DEBUG
    for (size_t i = 0; i < numofRows; i++)
    {
        std::cout << "debug: triggerNegative/triggerPositive Vector: " << i << ":" << result.triggerNegative[i] << "/" << result.triggerPositive[i] << " row inclusion: " << result.rowInclusion[i] << std::endl;
    }
#endif // DEBUG

    return result.rowInclusion;

}

std::vector<int> dataAnalysis::stabcPruning(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements, size_t& numofRows, int id) {
    dataAnalysis::DataInclusion result;
    double criteria = stabilityRequirements[id].stabilitychangeTrigger;
	double range = stabilityRequirements[id].stabilitychangeRange;
    size_t logsATrig = stabilityRequirements[id].logsafterTrigger;
    size_t logsATrig_do =-1;
    size_t logsBTrig = stabilityRequirements[id].logsbeforeTrigger;
    dataAnalysis::StabilityCheck stabilityCheck_i;
    stabilityCheck_i.stabcheckATrig = stabilityRequirements[id].stabilitywaitC;
    stabilityCheck_i.trigger = false;
	bool noCriteria = false;
    bool positvenegative = true;

   

    if (naCheckBool(range) || range == 0)
    {
        range = 0.0000000000000000000000000000000000000000001;
    }
    if (naCheckBool(stabilityRequirements[id].stabilitychangeTrigger))
    {


        criteria = channels[id].data[0];
		noCriteria = true;
#ifdef DEBUG
        std::cout << "debug: criteria = " << criteria << std::endl;
#endif // DEBUG
    }
    if (naCheckBool(logsATrig))
    {
        logsATrig = 0;
    }
    if (naCheckBool(logsBTrig))
    {
        logsBTrig = 0;
    }
    if (naCheckBool(stabilityCheck_i.stabcheckATrig))
    {
        stabilityCheck_i.stabcheckATrig = 0;
    }
    for (size_t i = 0; i < numofRows; i++)
    {
        result.id = id;
        result.rowInclusion.push_back(0);
        result.triggerNegative.push_back(0);
        result.triggerPositive.push_back(0);

    }
    
    for (size_t i = 1; i < numofRows; i++)
    {
        auto previous = channels[id].data[i - 1];
        auto current = channels[id].data[i];

        auto negative_trigger =[&positvenegative](auto current, auto previous, double criteria, double range) {
            return ((current > (criteria + range) || current < (criteria - range)) && positvenegative == false);
        };
        auto negative_no_criteria_trigger = [&positvenegative](auto current, auto previous, double criteria, double range) {
            return ((current > (criteria + range )|| current < (criteria - range)) && positvenegative == false);
            };
        auto positive_trigger =[&positvenegative](auto current, auto previous, double criteria, double range) {
            return (current >= (criteria - range) && current <= (criteria + range)) && positvenegative == true;
        };
		auto positve_no_criteria_trigger = [&positvenegative](auto current, auto previous, double criteria, double range) {
			return ((current > (criteria + range) || current < (criteria - range)) && positvenegative == true);
			};

        
        #ifdef DEBUG
        //std::cout << "debug: previous value at "<< i << " is " << previous << "and current is " << current << std::endl;
        #endif 
        try {
			
            
            if (noCriteria ? negative_no_criteria_trigger(current, previous, criteria, range) : negative_trigger(current, previous, criteria, range)) 
            
            {
                stabilityCheck_i.trigger = false;
                #ifdef DEBUG
				std::cout << "debug: trigger negative at " << i << std::endl;
                #endif // DEBUG
                positvenegative = true;
                
                result.triggerNegative.at(i) = 1;
                
               
            }
            else if (noCriteria ? positve_no_criteria_trigger(current, previous, criteria, range) : positive_trigger(current, previous, criteria, range)) {
                {
                    positvenegative = false;
                    stabilityCheck_i.trigger = false;
#ifdef DEBUG
                    std::cout << "debug: trigger postive at " << i << std::endl;
#endif // DEBUG
                    if (stabilityCheck_i.trigger == false)
                    {
#ifdef DEBUG
                        std::cout << "debug: stabilityCheck_i.activation set to " << i << std::endl;
#endif // DEBUG

                        stabilityCheck_i.activation = i;
                        stabilityCheck_i.trigger = true;
                    }
                    if (noCriteria)
                    {

                        criteria = channels[id].data[i];
#ifdef DEBUG
                        std::cout << "debug: criteira = : " << criteria << std::endl;
#endif // DEBUG


                    }

                    if (stabilityCheck_i.stabcheckATrig == 0)
                    {
                        if (logsBTrig)
                        {
                            result.triggerPositive.at(i - logsBTrig) = 1;
                        }
                        else
                        {
                            result.triggerPositive.at(i) = 1;
                        }
                    }

                    if (logsATrig)
                    {
#ifdef DEBUG

                        std::cout << "debug: logsATrig_do set to " << logsATrig + i + 1 << "logsAtrig = " << logsATrig << std::endl;
#endif // DEBUG
                        if (stabilityCheck_i.stabcheckATrig != 0)
                        {
                            logsATrig_do = logsATrig + i + stabilityCheck_i.stabcheckATrig + 1;
                        }
                        else
                        {
                            logsATrig_do = logsATrig + i + 1;

                        }
                    }
                }
            } 

            if (logsATrig_do == i)
            {
                for (size_t j = (i - 2); j > (i - 1 - logsATrig); j--)
                {
					result.triggerNegative.at(j) = 0;
                }
                #ifdef DEBUG
				std::cout << "debug: logsATrig_do at " << i << std::endl;
                #endif // DEBUG

                if(stabilityCheck_i.stabcheckATrig != 0)
                {
                    if  ( i >= (stabilityCheck_i.activation + stabilityCheck_i.stabcheckATrig))
                    {
                        #ifdef DEBUG
						std::cout << "debug trigger negative fired at " << i << std::endl;
                        #endif // DEBUG

                        

                        result.triggerNegative.at(i - 1) = 1;
                    }
                }
                else
                {
                 
                    
             
                    result.triggerNegative.at(i - 1) = 1;
                }
				logsATrig_do = -1;
            }
            stabilityCheck(stabilityCheck_i, result, i, logsBTrig);
            
        }

        catch (const std::out_of_range& ) {
            break;
        }

    }
    if (logsATrig_do != -1 && !naCheckBool(logsATrig))
    {
        result.triggerNegative.at(numofRows-1) = 1;
    }
    dataAnalysis::triggerFlip(result, numofRows, 0);
    #ifdef DEBUG
    for (size_t i = 0; i < numofRows; i++)
    {
        std::cout << "debug: triggerNegative/triggerPositive Vector: " << i << ":" << result.triggerNegative[i] << "/" << result.triggerPositive[i] << " row inclusion: " << result.rowInclusion[i] << std::endl;
    }
    #endif // DEBUG

	return result.rowInclusion;

}
std::vector<int> dataAnalysis::minmaxPruning(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements, size_t& numofRows, int id,bool minmax) {
    double criteria = minmax ? stabilityRequirements[id].max : stabilityRequirements[id].min;
    std::vector<int> rowInclusion;
    rowInclusion.reserve(numofRows);

    for (size_t i = 0; i < numofRows; i++)
    {
        auto current = channels[id].data[i];
        rowInclusion.push_back(minmax ? (current <= criteria) : (current >= criteria));
    }
#ifdef DEBUG
    for (size_t i = 0; i < numofRows; i++)
    {
		std::cout << "debug: rowInclusion Vector: " << i << ":" << rowInclusion[i] << std::endl;
    }
    std::cout << "debug: ran minmaxPruning" << std::endl;
#endif
    return rowInclusion;
}
std::vector<int> dataAnalysis::logicalandComparison(std::vector<std::vector<int>>& comparitor, size_t numofRows) {
    std::vector<int> results;
    for (size_t i = 0; i < numofRows; i++)
	{
		if (std::all_of(comparitor.begin(), comparitor.end(), [i](const std::vector<int>& value) { return value[i] == 1; })) {
			results.push_back(1);
		}
		else {
			results.push_back(0);
		} 
	}
    return results;
}

void dataAnalysis::dataPrune(const std::vector<dataLogger::Channel>& channels, std::vector<dataAnalysis::StabilityRequirement>& stabilityRequirements)
    {
    std::vector<std::vector<int>> results;
	dataAnalysis dataAnalysis_i;
	size_t numofRows = dataAnalysis_i.numofRows(channels);
    int i = 0;
    for (auto& elements : stabilityRequirements)
        {
        std::vector<std::vector<int>> results_i = {};
        if (!naCheckBool(elements.min)) 
            {
            results_i.push_back(dataAnalysis_i.minmaxPruning(channels, stabilityRequirements, numofRows, i, false));
            }
        if (!naCheckBool(elements.max))
        {
            results_i.push_back(dataAnalysis_i.minmaxPruning(channels, stabilityRequirements, numofRows, i, true));
        }
		if (!naCheckBool(elements.stabilitychangeTrigger) || !naCheckBool(elements.stabilitychangeRange) || !naCheckBool(elements.stabilitywaitC))
		{
			results_i.push_back(dataAnalysis_i.stabcPruning(channels, stabilityRequirements, numofRows, i));
		}
        if(!naCheckBool(elements.stabilityrocTrigger))
        {
            results_i.push_back(dataAnalysis_i.stabROCPruning(channels, stabilityRequirements, numofRows, i));
        }
		results.push_back(dataAnalysis_i.logicalandComparison(results_i, numofRows));
        i++;

        }
	dataAnalysis::rowInclusion = dataAnalysis_i.logicalandComparison(results, numofRows);
    #ifdef DEBUG
        for (size_t i = 0; i < numofRows; i++)
        {

            std::cout << "debug: rowInclusion Vector: " << i << ":" << dataAnalysis::rowInclusion[i] << std::endl;
        }
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
                << std::setw(tablewidth) << naCheckStr(stabilityRequirements[element.index].stabilitychangeTrigger)
                << std::setw(tablewidth) << naCheckStr(stabilityRequirements[element.index].stabilitychangeRange)
                << std::setw(tablewidth) << naCheckStr(stabilityRequirements[element.index].stabilitywaitC)
                << std::setw(tablewidth) << naCheckStr(stabilityRequirements[element.index].stabilityrocTrigger)
                << std::setw(tablewidth) << naCheckStr(stabilityRequirements[element.index].stabilitywaitROC)
                << std::setw(tablewidth) << naCheckStr(stabilityRequirements[element.index].min)
                << std::setw(tablewidth) << naCheckStr(stabilityRequirements[element.index].max)
                << std::setw(tablewidth) << naCheckStr(stabilityRequirements[element.index].logsbeforeTrigger)
                << std::setw(tablewidth) << naCheckStr(stabilityRequirements[element.index].logsafterTrigger)
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
            std::cout << "  The index of the channel you wish to configure, as shown in the table above.\n";
            std::cout << "  e.g. 7\n";
            std::cout << "\n";

            std::cout << "StabCTrig  (Stability Change Trigger)\n";
            std::cout << "  A threshold value. When the channel data crosses this value, a trigger is registered.\n";
            std::cout << "  Data capture begins immediately, or after WaitATrigC logs if set.\n";
            std::cout << "  Capture ends when the data crosses back over the threshold in the opposite direction.\n";
            std::cout << "  If no StabCTrig is set, the reference point defaults to the first value in the dataset\n";
            std::cout << "  or the first value following the end of the previous capture window.\n";
            std::cout << "  e.g. 1800 (rpm) — trigger registered when speed drops below 1800 rpm\n";
            std::cout << "\n";

            std::cout << "StabCRange  (Stability Change Range)\n";
            std::cout << "  An optional deadband applied around StabCTrig. Once capture has started, the channel\n";
            std::cout << "  must leave this range before capture can end. This prevents capture ending prematurely\n";
            std::cout << "  due to small oscillations around the threshold.\n";
            std::cout << "  e.g. 10 (rpm) — once triggered at 1800 rpm, capture will not end until the value\n";
            std::cout << "  returns above 1810 rpm.\n";
            std::cout << "\n";

            std::cout << "WaitATrigC  (Wait After Trigger - Change)\n";
            std::cout << "  Delays the start of data capture after a StabCTrig activation. Once the trigger fires,\n";
            std::cout << "  the tool will wait this many logs before beginning to capture data.\n";
            std::cout << "  e.g. 30 — data capture begins 30 logs after the StabCTrig threshold is crossed.\n";
            std::cout << "\n";

            std::cout << "StabROCTrig  (Stability Rate-of-Change Trigger)\n";
            std::cout << "  A rate-of-change threshold evaluated over a rolling 10-log window (first log in,\n";
            std::cout << "  first log out). When the rate of change between the first and last log in the window\n";
            std::cout << "  meets or exceeds this value, a trigger is registered. Data capture begins immediately,\n";
            std::cout << "  or after WaitATrigROC logs if set.\n";
            std::cout << "  Value is set per log: So for 10Hz data a rate of -150 rpm/s, a input value of -15 would correspond to a 150 rpm drop over 1s\n";
          
            std::cout << "  Both positive and negative rates of change are supported.\n";
            std::cout << "  Note: StabROCTrig activations are typically short-lived. It is strongly recommended\n";
            std::cout << "  to pair this with WaitATrigROC to capture a meaningful window of data.\n";
            std::cout << "  e.g. -15 — triggers when engine speed falls by more than 150 rpm over 10 logs (1Hz data)\n";
            std::cout << "\n";

            std::cout << "WaitATrigROC  (Wait After Trigger - Rate of Change)\n";
            std::cout << "  Delays the start of data capture after a StabROCTrig activation. Once the trigger fires,\n";
            std::cout << "  the tool will wait this many logs before beginning to capture data.\n";
            std::cout << "  e.g. 50 — data capture begins 50 logs after the StabROCTrig threshold is met.\n";
            std::cout << "\n";

            std::cout << "Min / Max  (Passive Filter)\n";
            std::cout << "  Passive filters applied independently of all trigger categories. Any data row where\n";
            std::cout << "  the channel value falls outside the Min/Max range will be excluded from the output.\n";
            std::cout << "  Min and Max can be set independently of each other.\n";
            std::cout << "  e.g. Min 600, Max 800 (kPa) — only rows where the channel reads between 600-800 kPa\n";
            std::cout << "  will be retained.\n";
            std::cout << "\n";

            std::cout << "LogsBTrig  (Logs Before Trigger)\n";
            std::cout << "  The number of logs to include before the start of any capture window.\n";
            std::cout << "  e.g. 10 — the 10 logs immediately preceding the trigger activation are included.\n";
            std::cout << "\n";

            std::cout << "LogsATrig  (Logs After Trigger End - Overruling)\n";
            std::cout << "  An overruling end condition. Once a capture window has started, capture will end\n";
            std::cout << "  after this many logs regardless of whether any trigger end condition has been met.\n";
            std::cout << "  This applies across all trigger categories on the channel.\n";
            std::cout << "  e.g. 100 — capture ends after 100 logs from the start of the capture window.\n";
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
                        stabilityRequirements[id_int].logsafterTrigger = std::stoi(string);
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
















