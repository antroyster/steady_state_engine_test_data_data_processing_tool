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
#include <cmath>

// Returns "N/A" if value is the "unset" sentinel for its type (NaN for
// floating point fields, -1 for integer fields), otherwise the value as text.
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
// True if value is the "unset" sentinel for its type (NaN for floating
// point fields, -1 for integer fields).
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

    // Per-channel configuration for the four independent pruning features.
    // A field left at its default (NaN for doubles, -1 for ints) is
    // "unset" and its feature is skipped for that channel. See the
    // in-app help (promptStabilityRequirements, option "I") for the full
    // description of each field's behavior.
    struct StabilityRequirement {
        // StabC: logs the data must stay inside [stabCTriggerValue +/- stabCRangeBand]
        // before the capture start is confirmed. 0/unset = confirm immediately.
        int stabCStartConfirmLogs = -1;
        // StabROC: number of logs spanned when computing the rate of change
        // (current value vs. the value this many logs ago).
        int stabROCWindowLogs = -1;
        // StabC: center of the target band. If unset, the tool instead
        // tracks a self-resetting reference (see stabcPruning).
        double stabCTriggerValue = std::numeric_limits<double>::quiet_NaN();
        // StabC: +/- half-width of the target band around stabCTriggerValue.
        double stabCRangeBand = std::numeric_limits<double>::quiet_NaN();
        // StabROC: rate-of-change threshold, in value per log.
        double stabROCTriggerRate = std::numeric_limits<double>::quiet_NaN();
        // Passive filter: minimum acceptable value for the channel.
        double minValue = std::numeric_limits<double>::quiet_NaN();
        // Passive filter: maximum acceptable value for the channel.
        double maxValue = std::numeric_limits<double>::quiet_NaN();
        // StabC/StabROC: number of logs to include before a capture start.
        int logsBeforeTrigger = -1;
        // StabC/StabROC: overruling end - capture ends this many logs
        // after it started, regardless of any other end condition.
        int logsAfterTrigger = -1;
    };

    // Working state built up by stabcPruning/stabROCPruning for one channel:
    // per-row flags marking where a capture window starts/ends, and the
    // final per-row inclusion result derived from those flags.
    struct DataInclusion {
        std::vector<int> captureStartFlags; // 1 on the row a capture window starts
        std::vector<int> captureEndFlags;   // 1 on the row a capture window ends (inclusive)
        int id;
        std::vector<int> rowInclusion;      // final result: 1 = row is kept
    };

    // Tracks a StabC capture start that has been detected but not yet
    // confirmed, while waiting out stabCStartConfirmLogs to make sure the
    // data doesn't leave the target band again before confirming.
    struct StabCPendingStart {
        size_t entryIndex;       // row index where the band was entered
        size_t confirmWaitLogs;  // logs required in-band before confirming (copy of stabCStartConfirmLogs)
        bool awaitingConfirmation;
    };

    // If pendingStart.awaitingConfirmation has survived confirmWaitLogs
    // logs without being cancelled (cancellation happens in stabcPruning
    // when the data leaves the band early), confirms the capture start by
    // marking captureStartFlags at the correct row (adjusted for
    // logsBeforeTrigger).
    void confirmStabCStart(dataAnalysis::StabCPendingStart& pendingStart, dataAnalysis::DataInclusion& inclusion, size_t& rowIndex, size_t& logsBeforeTrigger);

    // Fills stabilityRequirements with one default-constructed (all
    // fields unset) entry per channel.
    void initializeStabilityRequirements(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements);

    // Runs every configured pruning feature for every channel and combines
    // the results (a row is kept only if every configured feature, on
    // every channel, agrees to keep it) into dataAnalysis::rowInclusion.
    void dataPrune(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements);

    // Interactive console loop for entering/reviewing per-channel
    // StabilityRequirement values. Type "I" at the prompt for full
    // documentation of every field, "E" to finish and continue.
    void promptStabilityRequirements(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements);

    // "Stability Change" pruning: keeps rows while the channel is inside
    // [stabCTriggerValue +/- stabCRangeBand]. If stabCTriggerValue is
    // unset, the band instead follows a self-resetting reference that
    // starts at the channel's first value and re-centers on every
    // detected step change - i.e. a general "is this channel currently
    // stable" check rather than a fixed target band.
    std::vector<int> stabcPruning(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements, size_t& numofRows, int id);

    // Passive per-row filter: keeps rows where the channel is >= minValue
    // (minmax=false) or <= maxValue (minmax=true). Set both minValue and
    // maxValue (via two calls) to keep a channel between the two.
    std::vector<int> minmaxPruning(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements, size_t& numofRows, int id, bool minmax);

    // "Stability Rate-of-Change" pruning: keeps a window of rows starting
    // whenever the channel's rate of change (over stabROCWindowLogs logs)
    // meets or exceeds stabROCTriggerRate.
    std::vector<int> stabROCPruning(const std::vector<dataLogger::Channel>& channels, std::vector<StabilityRequirement>& stabilityRequirements, size_t& numofRows, int id);

    // Number of rows in the longest channel.
    size_t numofRows(const std::vector<dataLogger::Channel>& channels);

    // Expands a DataInclusion's captureStartFlags/captureEndFlags into its
    // rowInclusion vector: every row from a start flag through its
    // matching end flag (inclusive) is marked included.
    // invertInclusion flips which state ("inside a window" vs "outside
    // one") counts as included; every current caller passes false.
    void expandCaptureWindows(dataAnalysis::DataInclusion& inclusion, size_t& numofRows, bool invertInclusion);

    // Row i is 1 only if every vector in `vectors` has a 1 at row i.
    std::vector<int> logicalAndCombine(std::vector<std::vector<int>>& vectors, size_t numofRows);

    std::vector<StabilityRequirement> stabilityRequirements;
    std::vector<int> rowInclusion;
private:

};
#endif // DATAANALYSIS_H
