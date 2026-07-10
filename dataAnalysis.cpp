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

void dataAnalysis::initializeStabilityRequirements(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements) {
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
    auto longestChannel = std::max_element(
        channels.begin(),
        channels.end(),
        [](const dataLogger::Channel& a, const dataLogger::Channel& b)
        {return a.data.size() < b.data.size();}
        );
    return longestChannel->data.size();
}
void dataAnalysis::expandCaptureWindows(dataAnalysis::DataInclusion& inclusion, size_t& numofRows, bool invertInclusion)
{
    bool insideCaptureWindow = 0;
    for (size_t i = 0; i < numofRows; i++) {
        if (insideCaptureWindow == 0)
        {
            if (inclusion.captureStartFlags[i] == 1)
            {
                insideCaptureWindow = 1;
                inclusion.rowInclusion[i] = !invertInclusion;
            }
        }
        if (insideCaptureWindow == 1)
        {
            inclusion.rowInclusion[i] = !invertInclusion;
            if (inclusion.captureEndFlags[i] == 1)
            {
                insideCaptureWindow = 0;
            }

        }

    }
}
void dataAnalysis::confirmStabCStart(dataAnalysis::StabCPendingStart& pendingStart, dataAnalysis::DataInclusion& inclusion, size_t& rowIndex, size_t& logsBeforeTrigger)
{
#ifdef DEBUG
    std::cout << "Debug: stability check values" << "current : " << static_cast<int>(rowIndex) << " confirmWaitLogs: " << static_cast<int>(pendingStart.confirmWaitLogs) << " entryIndex: " << static_cast<int>(pendingStart.entryIndex) << " awaitingConfirmation: " << static_cast<int>(pendingStart.awaitingConfirmation) << std::endl;
#endif // DEBUG

    if (pendingStart.awaitingConfirmation && ( rowIndex >= (pendingStart.entryIndex+pendingStart.confirmWaitLogs)) )
    {
        #ifdef DEBUG
        std::cout << "debug:  has gone beyond stability threshold at " << rowIndex << std::endl;
        #endif // DEBUG

        for (size_t j = (rowIndex - 2); j > (rowIndex - 1 - pendingStart.confirmWaitLogs); j--)
        {

            inclusion.captureStartFlags.at(j) = 0;

        }
        if (logsBeforeTrigger)
        {
            inclusion.captureStartFlags.at(rowIndex - logsBeforeTrigger) = 1;
        }
        else
        {
            inclusion.captureStartFlags.at(rowIndex) = 1;
        }
        pendingStart.awaitingConfirmation = false;
    }
}
std::vector<int> dataAnalysis::stabROCPruning(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements, size_t& numofRows, int id) {
    dataAnalysis::DataInclusion inclusion;
    double triggerRate = stabilityRequirements[id].stabROCTriggerRate;
    size_t windowLogs = stabilityRequirements[id].stabROCWindowLogs;
    size_t logsAfterTrigger = stabilityRequirements[id].logsAfterTrigger;
    size_t overrulingEndRow = -1;
    size_t logsBeforeTrigger = stabilityRequirements[id].logsBeforeTrigger;
    bool awaitingTriggerEvent = true;
    bool triggerRateIsPositive = (triggerRate >= 0) ? true : false;


    if (naCheckBool(logsAfterTrigger))
    {
        logsAfterTrigger = 0;
    }
    if (naCheckBool(logsBeforeTrigger))
    {
        logsBeforeTrigger = 0;
    }
    if (naCheckBool(windowLogs))
    {
        windowLogs = 0;
    }
    for (size_t i = 0; i < numofRows; i++)
    {
        inclusion.id = id;
        inclusion.rowInclusion.push_back(0);
        inclusion.captureEndFlags.push_back(0);
        inclusion.captureStartFlags.push_back(0);



    }
#ifdef DEBUG
    std::cout << "debug: generated result vector" << std::endl;
#endif // DEBUG


    for (size_t i = windowLogs; i < (numofRows); i++)
    {
        try {
            auto current = channels[id].data[i];
            auto previous = channels[id].data[i - windowLogs];
            auto rateOfChange = (current - previous) / windowLogs;
            bool currentRateIsPositive = (rateOfChange >= 0) ? true : false;
#ifdef DEBUG
            if (i > 1300 || i < 20)
            {
                std::cout << "debug: current value at " << i << " is " << current << " and previous value at " << previous << std::endl;
                std::cout << "debug: rate of change at " << i << " is " << rateOfChange << " and triggerRate is " << triggerRate << std::endl;
                std::cout << "debug: currentRateIsPositive is " << currentRateIsPositive << " and triggerRateIsPositive is " << triggerRateIsPositive << std::endl;

            }
#endif // DEBUG

            if ((std::abs(rateOfChange) >= std::abs(triggerRate)) && awaitingTriggerEvent == true)
            {


                if ((triggerRateIsPositive != currentRateIsPositive))
                {
                    continue;
                }
                awaitingTriggerEvent = false;
#ifdef DEBUG
                std::cout << "debug: awaitingTriggerEvent set to " << awaitingTriggerEvent << " at row " << i << std::endl;
#endif // DEBUG
                if (logsBeforeTrigger)
                {

                    inclusion.captureStartFlags.at(i+1 - logsBeforeTrigger) = 1;
                }
                else
                {
                    inclusion.captureStartFlags.at(i+1) = 1;
                }

                if (logsAfterTrigger)
                {
                    overrulingEndRow = logsAfterTrigger + i + 1;
                }

            }
            else if (awaitingTriggerEvent == false)
            {
                if ((std::abs(rateOfChange) <= std::abs(triggerRate)) && (triggerRateIsPositive == currentRateIsPositive))
                {

                    awaitingTriggerEvent = true;
#ifdef DEBUG
                    std::cout << "debug: awaitingTriggerEvent set to " << awaitingTriggerEvent << " at row " << i << std::endl;
#endif // DEBUG
                    inclusion.captureEndFlags.at(i) = 1;
                }
                else if (triggerRateIsPositive != currentRateIsPositive)
                {
                    awaitingTriggerEvent = true;
#ifdef DEBUG
                    std::cout << "debug: awaitingTriggerEvent set to " << awaitingTriggerEvent << " at row " << i << std::endl;
#endif // DEBUG
                    inclusion.captureEndFlags.at(i) = 1;
                }

            }
            if (overrulingEndRow == i)
            {
                for (size_t j = (i - 2); j > (i - 1 - logsAfterTrigger); j--)
                {
                    inclusion.captureEndFlags.at(j) = 0;
                }
                inclusion.captureEndFlags.at(i - 1) = 1;
                awaitingTriggerEvent = true;
#ifdef DEBUG
                std::cout << "debug: awaitingTriggerEvent set to " << awaitingTriggerEvent << " at row " << i << std::endl;
#endif // DEBUG
                overrulingEndRow = -1;
            }
		}
        catch (const std::out_of_range&) {
            std::cout << "debug: out of range exception at " << i << std::endl;
            break;
        }
    }
    if (overrulingEndRow != -1 && !naCheckBool(logsAfterTrigger))
    {
        inclusion.captureEndFlags.at(numofRows - 1) = 1;
    }
    dataAnalysis::expandCaptureWindows(inclusion, numofRows, 0);
#ifdef DEBUG
    for (size_t i = 0; i < numofRows; i++)
    {
        std::cout << "debug: captureEndFlags/captureStartFlags Vector: " << i << ":" << inclusion.captureEndFlags[i] << "/" << inclusion.captureStartFlags[i] << " row inclusion: " << inclusion.rowInclusion[i] << std::endl;
    }
#endif // DEBUG

    return inclusion.rowInclusion;

}

std::vector<int> dataAnalysis::stabcPruning(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements, size_t& numofRows, int id) {
    dataAnalysis::DataInclusion inclusion;
    double triggerValue = stabilityRequirements[id].stabCTriggerValue;
	double rangeBand = stabilityRequirements[id].stabCRangeBand;
    size_t logsAfterTrigger = stabilityRequirements[id].logsAfterTrigger;
    size_t overrulingEndRow =-1;
    size_t logsBeforeTrigger = stabilityRequirements[id].logsBeforeTrigger;
    dataAnalysis::StabCPendingStart pendingStart;
    pendingStart.confirmWaitLogs = stabilityRequirements[id].stabCStartConfirmLogs;
    pendingStart.awaitingConfirmation = false;
	bool dynamicTrigger = false;
    bool awaitingBandEntry = true;



    if (naCheckBool(rangeBand) || rangeBand == 0)
    {
        rangeBand = 0.0000000000000000000000000000000000000000001;
    }
    if (naCheckBool(stabilityRequirements[id].stabCTriggerValue))
    {


        triggerValue = channels[id].data[0];
		dynamicTrigger = true;
#ifdef DEBUG
        std::cout << "debug: triggerValue = " << triggerValue << std::endl;
#endif // DEBUG
    }
    if (naCheckBool(logsAfterTrigger))
    {
        logsAfterTrigger = 0;
    }
    if (naCheckBool(logsBeforeTrigger))
    {
        logsBeforeTrigger = 0;
    }
    if (naCheckBool(pendingStart.confirmWaitLogs))
    {
        pendingStart.confirmWaitLogs = 0;
    }
    for (size_t i = 0; i < numofRows; i++)
    {
        inclusion.id = id;
        inclusion.rowInclusion.push_back(0);
        inclusion.captureEndFlags.push_back(0);
        inclusion.captureStartFlags.push_back(0);

    }

    for (size_t i = 1; i < numofRows; i++)
    {
        auto previous = channels[id].data[i - 1];
        auto current = channels[id].data[i];

        // Fixed-target-band mode (dynamicTrigger == false): "entering"/
        // "exiting" are level checks against [triggerValue +/- rangeBand] -
        // the row is either inside the band or it isn't, regardless of
        // which direction it approached from.
        auto exitsFixedBand =[&awaitingBandEntry](auto current, auto previous, double triggerValue, double rangeBand) {
            return ((current > (triggerValue + rangeBand) || current < (triggerValue - rangeBand)) && awaitingBandEntry == false);
        };
        auto entersFixedBand =[&awaitingBandEntry](auto current, auto previous, double triggerValue, double rangeBand) {
            return (current >= (triggerValue - rangeBand) && current <= (triggerValue + rangeBand)) && awaitingBandEntry == true;
        };
        // Self-referencing mode (dynamicTrigger == true): both lambdas
        // share the same condition (current has drifted outside the
        // reference band) - which one applies depends only on whether we
        // were still awaiting the first drift (re-anchor to the new value
        // and start a window) or already inside a captured window
        // (end it, since the reference has moved again).
        auto driftsFromReferenceEnd = [&awaitingBandEntry](auto current, auto previous, double triggerValue, double rangeBand) {
            return ((current > (triggerValue + rangeBand )|| current < (triggerValue - rangeBand)) && awaitingBandEntry == false);
            };
		auto driftsFromReferenceRearm = [&awaitingBandEntry](auto current, auto previous, double triggerValue, double rangeBand) {
			return ((current > (triggerValue + rangeBand) || current < (triggerValue - rangeBand)) && awaitingBandEntry == true);
			};


        #ifdef DEBUG
        //std::cout << "debug: previous value at "<< i << " is " << previous << "and current is " << current << std::endl;
        #endif
        try {


            if (dynamicTrigger ? driftsFromReferenceEnd(current, previous, triggerValue, rangeBand) : exitsFixedBand(current, previous, triggerValue, rangeBand))

            {
                pendingStart.awaitingConfirmation = false;
                #ifdef DEBUG
				std::cout << "debug: trigger negative at " << i << std::endl;
                #endif // DEBUG
                awaitingBandEntry = true;

                inclusion.captureEndFlags.at(i) = 1;


            }
            else if (dynamicTrigger ? driftsFromReferenceRearm(current, previous, triggerValue, rangeBand) : entersFixedBand(current, previous, triggerValue, rangeBand)) {
                {
                    awaitingBandEntry = false;
                    pendingStart.awaitingConfirmation = false;
#ifdef DEBUG
                    std::cout << "debug: trigger postive at " << i << std::endl;
#endif // DEBUG
                    if (pendingStart.awaitingConfirmation == false)
                    {
#ifdef DEBUG
                        std::cout << "debug: pendingStart.entryIndex set to " << i << std::endl;
#endif // DEBUG

                        pendingStart.entryIndex = i;
                        pendingStart.awaitingConfirmation = true;
                    }
                    if (dynamicTrigger)
                    {

                        triggerValue = channels[id].data[i];
#ifdef DEBUG
                        std::cout << "debug: triggerValue = : " << triggerValue << std::endl;
#endif // DEBUG


                    }

                    if (pendingStart.confirmWaitLogs == 0)
                    {
                        if (logsBeforeTrigger)
                        {
                            inclusion.captureStartFlags.at(i - logsBeforeTrigger) = 1;
                        }
                        else
                        {
                            inclusion.captureStartFlags.at(i) = 1;
                        }
                    }

                    if (logsAfterTrigger)
                    {
#ifdef DEBUG

                        std::cout << "debug: overrulingEndRow set to " << logsAfterTrigger + i + 1 << "logsAfterTrigger = " << logsAfterTrigger << std::endl;
#endif // DEBUG
                        if (pendingStart.confirmWaitLogs != 0)
                        {
                            overrulingEndRow = logsAfterTrigger + i + pendingStart.confirmWaitLogs + 1;
                        }
                        else
                        {
                            overrulingEndRow = logsAfterTrigger + i + 1;

                        }
                    }
                }
            }

            if (overrulingEndRow == i)
            {
                for (size_t j = (i - 2); j > (i - 1 - logsAfterTrigger); j--)
                {
					inclusion.captureEndFlags.at(j) = 0;
                }
                #ifdef DEBUG
				std::cout << "debug: overrulingEndRow at " << i << std::endl;
                #endif // DEBUG

                if(pendingStart.confirmWaitLogs != 0)
                {
                    if  ( i >= (pendingStart.entryIndex + pendingStart.confirmWaitLogs))
                    {
                        #ifdef DEBUG
						std::cout << "debug trigger negative fired at " << i << std::endl;
                        #endif // DEBUG



                        inclusion.captureEndFlags.at(i - 1) = 1;
                    }
                }
                else
                {



                    inclusion.captureEndFlags.at(i - 1) = 1;
                }
				overrulingEndRow = -1;
            }
            confirmStabCStart(pendingStart, inclusion, i, logsBeforeTrigger);

        }

        catch (const std::out_of_range& ) {
            break;
        }

    }
    if (overrulingEndRow != -1 && !naCheckBool(logsAfterTrigger))
    {
        inclusion.captureEndFlags.at(numofRows-1) = 1;
    }
    dataAnalysis::expandCaptureWindows(inclusion, numofRows, 0);
    #ifdef DEBUG
    for (size_t i = 0; i < numofRows; i++)
    {
        std::cout << "debug: captureEndFlags/captureStartFlags Vector: " << i << ":" << inclusion.captureEndFlags[i] << "/" << inclusion.captureStartFlags[i] << " row inclusion: " << inclusion.rowInclusion[i] << std::endl;
    }
    #endif // DEBUG

	return inclusion.rowInclusion;

}
std::vector<int> dataAnalysis::minmaxPruning(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements, size_t& numofRows, int id,bool minmax) {
    double limitValue = minmax ? stabilityRequirements[id].maxValue : stabilityRequirements[id].minValue;
    std::vector<int> rowInclusion;
    rowInclusion.reserve(numofRows);

    for (size_t i = 0; i < numofRows; i++)
    {
        auto current = channels[id].data[i];
        rowInclusion.push_back(minmax ? (current <= limitValue) : (current >= limitValue));
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
std::vector<int> dataAnalysis::logicalAndCombine(std::vector<std::vector<int>>& vectors, size_t numofRows) {
    std::vector<int> results;
    for (size_t i = 0; i < numofRows; i++)
	{
		if (std::all_of(vectors.begin(), vectors.end(), [i](const std::vector<int>& value) { return value[i] == 1; })) {
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
    std::vector<std::vector<int>> allChannelResults;
	dataAnalysis dataAnalysis_i;
	size_t numofRows = dataAnalysis_i.numofRows(channels);
    int channelId = 0;
    for (auto& requirement : stabilityRequirements)
        {
        std::vector<std::vector<int>> channelResults = {};
        if (!naCheckBool(requirement.minValue))
            {
            channelResults.push_back(dataAnalysis_i.minmaxPruning(channels, stabilityRequirements, numofRows, channelId, false));
            }
        if (!naCheckBool(requirement.maxValue))
        {
            channelResults.push_back(dataAnalysis_i.minmaxPruning(channels, stabilityRequirements, numofRows, channelId, true));
        }
		if (!naCheckBool(requirement.stabCTriggerValue) || !naCheckBool(requirement.stabCRangeBand) || !naCheckBool(requirement.stabCStartConfirmLogs))
		{
			channelResults.push_back(dataAnalysis_i.stabcPruning(channels, stabilityRequirements, numofRows, channelId));
		}
        if(!naCheckBool(requirement.stabROCTriggerRate))
        {
            channelResults.push_back(dataAnalysis_i.stabROCPruning(channels, stabilityRequirements, numofRows, channelId));
        }
		allChannelResults.push_back(dataAnalysis_i.logicalAndCombine(channelResults, numofRows));
        channelId++;

        }
	dataAnalysis::rowInclusion = dataAnalysis_i.logicalAndCombine(allChannelResults, numofRows);
    #ifdef DEBUG
        for (size_t i = 0; i < numofRows; i++)
        {

            std::cout << "debug: rowInclusion Vector: " << i << ":" << dataAnalysis::rowInclusion[i] << std::endl;
        }
    #endif // DEBUG

    }

void dataAnalysis::promptStabilityRequirements(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements) {
    int columnWidth = 12;
    std::string inputLine;
    std::string idToken;
    int channelId;
    bool shouldExit = false;
    auto truncate = [columnWidth](const std::string& input) {
        return input.size() > columnWidth ? input.substr(0, columnWidth - 1) : input;
        };
    while (!shouldExit) {
        std::cout << std::left
            << std::setw(columnWidth) << "ID"
            << std::setw(columnWidth) << "Channel"
            << std::setw(columnWidth) << "Units"
            << std::setw(columnWidth) << "StabCTrig"
            << std::setw(columnWidth) << "StabCRange"
            << std::setw(columnWidth) << "WaitATrigC"
            << std::setw(columnWidth) << "StabROCTrig"
            << std::setw(columnWidth) << "ROCWindow"
            << std::setw(columnWidth) << "Min"
            << std::setw(columnWidth) << "Max"
            << std::setw(columnWidth) << "LogsBTrig"
            << std::setw(columnWidth) << "LogsATrig"
            << "\n";
        std::cout << std::string(columnWidth * 12, '-') << "\n";
        for (const auto& element : channels) {
            std::cout << std::left
                << std::setw(columnWidth) << element.index
                << std::setw(columnWidth) << truncate(element.id)
                << std::setw(columnWidth) << truncate(element.units)
                << std::setw(columnWidth) << naCheckStr(stabilityRequirements[element.index].stabCTriggerValue)
                << std::setw(columnWidth) << naCheckStr(stabilityRequirements[element.index].stabCRangeBand)
                << std::setw(columnWidth) << naCheckStr(stabilityRequirements[element.index].stabCStartConfirmLogs)
                << std::setw(columnWidth) << naCheckStr(stabilityRequirements[element.index].stabROCTriggerRate)
                << std::setw(columnWidth) << naCheckStr(stabilityRequirements[element.index].stabROCWindowLogs)
                << std::setw(columnWidth) << naCheckStr(stabilityRequirements[element.index].minValue)
                << std::setw(columnWidth) << naCheckStr(stabilityRequirements[element.index].maxValue)
                << std::setw(columnWidth) << naCheckStr(stabilityRequirements[element.index].logsBeforeTrigger)
                << std::setw(columnWidth) << naCheckStr(stabilityRequirements[element.index].logsAfterTrigger)
                << "\n";
        }
        std::cout << "Please Add Numerical Stability Requirements in the format \"ID:StabCTrig:StabCRange:WaitATrigC:StabROCTrig:ROCWindow:Min:Max:LogsBTrig:LogsATrig\"" << std::endl;
        std::cout << "For Information on the required input format type I , to exit type E" << std::endl;
        std::cin >> inputLine;
		if (inputLine == "I" || inputLine == "i") {
            std::string pauseInput;
            std::cout << "Input format: ID:StabCTrig:StabCRange:WaitATrigC:StabROCTrig:ROCWindow:Min:Max:LogsBTrig:LogsATrig\n";
            std::cout << "To leave a parameter unset, leave it blank: e.g. 2:1800:10:30:-15:10:::10:10\n";
            std::cout << "\n";

            std::cout << "--- REFERENCES ---\n";
            std::cout << "\n";

            std::cout << "ID\n";
            std::cout << "  The index of the channel you wish to configure, as shown in the table above.\n";
            std::cout << "  e.g. 7\n";
            std::cout << "\n";

            std::cout << "StabCTrig  (Stability Change Trigger)\n";
            std::cout << "  Center of a target band: [StabCTrig - StabCRange, StabCTrig + StabCRange].\n";
            std::cout << "  Capture starts on the row the channel enters this band (from either direction)\n";
            std::cout << "  and ends on the row it leaves the band again (either direction). If WaitATrigC\n";
            std::cout << "  is set, entering the band only starts a *pending* capture - it's confirmed (and\n";
            std::cout << "  backdated to the entry row) only if the channel stays inside the band for the\n";
            std::cout << "  full WaitATrigC logs; leaving early cancels it with no capture recorded.\n";
            std::cout << "  If StabCTrig is left unset, there is no fixed band. Instead the tool tracks a\n";
            std::cout << "  self-resetting reference: it starts at the channel's first value, and every time\n";
            std::cout << "  the channel drifts more than StabCRange away from the current reference, that\n";
            std::cout << "  drift is captured and the reference re-centers on the new value. This turns it\n";
            std::cout << "  into a general \"is this channel currently stable\" check with no target value.\n";
            std::cout << "  e.g. 1800 (rpm), StabCRange 10 — capture whenever engine speed is within\n";
            std::cout << "  1790-1810 rpm, from whichever direction it arrives.\n";
            std::cout << "\n";

            std::cout << "StabCRange  (Stability Change Range)\n";
            std::cout << "  Half-width of the StabCTrig target band (or, with no StabCTrig, the tolerance\n";
            std::cout << "  around the self-resetting reference). Required for StabC to do anything -\n";
            std::cout << "  without it there is no band to check membership against.\n";
            std::cout << "  e.g. 10 (rpm) with StabCTrig 1800 — the target band is 1790-1810 rpm.\n";
            std::cout << "\n";

            std::cout << "WaitATrigC  (Wait to Confirm Trigger - Change)\n";
            std::cout << "  Number of logs the channel must remain inside the StabC target band before the\n";
            std::cout << "  capture start is confirmed. If the channel leaves the band before this many logs\n";
            std::cout << "  have passed, the pending capture is cancelled entirely (nothing is captured for\n";
            std::cout << "  that entry). Once confirmed, the capture is backdated to the row the band was\n";
            std::cout << "  first entered (plus LogsBTrig, if set). Leave unset/0 to confirm immediately.\n";
            std::cout << "  e.g. 30 — the channel must sit inside the band for 30 straight logs before that\n";
            std::cout << "  dwell counts as captured.\n";
            std::cout << "\n";

            std::cout << "StabROCTrig  (Stability Rate-of-Change Trigger)\n";
            std::cout << "  A rate-of-change threshold evaluated over a ROCWindow-log window (current value\n";
            std::cout << "  vs. the value ROCWindow logs earlier). When that rate meets or exceeds this\n";
            std::cout << "  value (matching its sign), a trigger is registered and capture starts, or after\n";
            std::cout << "  WaitATrigROC logs if set. Value is set per log: e.g. for 10Hz data, a rate of\n";
            std::cout << "  -150 rpm/s over a 10-log window corresponds to a per-log value of -15.\n";
            std::cout << "  Both positive and negative rates of change are supported.\n";
            std::cout << "  Note: StabROCTrig activations are typically short-lived. It is strongly\n";
            std::cout << "  recommended to pair this with LogsATrig to capture a meaningful window of data.\n";
            std::cout << "  e.g. -15 with ROCWindow 10 — triggers when engine speed falls by more than\n";
            std::cout << "  150 rpm over 10 logs.\n";
            std::cout << "\n";

            std::cout << "ROCWindow  (Rate-of-Change Window)\n";
            std::cout << "  Number of logs spanned when computing the rate of change for StabROCTrig: the\n";
            std::cout << "  rate is (current value - value this many logs ago) / this many logs.\n";
            std::cout << "  e.g. 10 — rate of change is measured across a rolling 10-log window.\n";
            std::cout << "\n";

            std::cout << "Min / Max  (Passive Filter)\n";
            std::cout << "  A static per-row filter applied independently of all other categories, on this\n";
            std::cout << "  channel alone. Any row where the channel value falls outside the Min/Max range\n";
            std::cout << "  is excluded from the output, regardless of any StabC/StabROC capture window.\n";
            std::cout << "  Min and Max can be set independently of each other.\n";
            std::cout << "  e.g. Min 600, Max 800 (kPa) — only rows where the channel reads between 600-800\n";
            std::cout << "  kPa will be retained.\n";
            std::cout << "\n";

            std::cout << "LogsBTrig  (Logs Before Trigger)\n";
            std::cout << "  The number of logs to include before the start of any capture window (StabC or\n";
            std::cout << "  StabROC). Applied by backdating the captured start row.\n";
            std::cout << "  e.g. 10 — the 10 logs immediately preceding the trigger activation are included.\n";
            std::cout << "\n";

            std::cout << "LogsATrig  (Logs After Trigger Start - Overruling End)\n";
            std::cout << "  An overruling end condition. Once a capture window has started (StabC or\n";
            std::cout << "  StabROC), capture ends after this many logs regardless of whether any other end\n";
            std::cout << "  condition has been met. Applies across both trigger categories on the channel.\n";
            std::cout << "  e.g. 100 — capture ends after 100 logs from the start of the capture window.\n";
            std::cout << "\n";
            std::cout << " Type any Character + Enter Key to Continue" <<std::endl;
            std::cin >> pauseInput;

			continue;
		}
		else if (inputLine == "E" || inputLine == "e"){
            shouldExit = true;
			break;
		}
        else
        {
            try {
                std::stringstream sInputLine(inputLine);
                std::getline(sInputLine, idToken, ':');
                channelId = std::stoi(idToken);
                int fieldIndex = 1;
                std::string token;
                while (std::getline(sInputLine, token, ':')) {
                    if (token.empty() || token == "\n" ) {
                        fieldIndex++;
                        continue;
                     }
                    switch (fieldIndex) {
                    case 1:
                        stabilityRequirements[channelId].stabCTriggerValue = std::stod(token);
                        fieldIndex++;
                        break;
                    case 2:
                        stabilityRequirements[channelId].stabCRangeBand = std::stod(token);
                        fieldIndex++;
                        break;
                    case 3:
                        stabilityRequirements[channelId].stabCStartConfirmLogs = std::stoi(token);
                        fieldIndex++;
                        break;
                    case 4:
                        stabilityRequirements[channelId].stabROCTriggerRate = std::stod(token);
                        fieldIndex++;
                        break;
                    case 5:
                        stabilityRequirements[channelId].stabROCWindowLogs = std::stoi(token);
                        fieldIndex++;
                        break;
                    case 6:
                        stabilityRequirements[channelId].minValue = std::stod(token);
                        fieldIndex++;
                        break;

                    case 7:
                        stabilityRequirements[channelId].maxValue = std::stod(token);
                        fieldIndex++;
                        break;
                    case 8:
                        stabilityRequirements[channelId].logsBeforeTrigger = std::stoi(token);
                        fieldIndex++;
                        break;
                    case 9:
                        stabilityRequirements[channelId].logsAfterTrigger = std::stoi(token);
                        fieldIndex++;
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
