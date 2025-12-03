#pragma once

#include "../../cpptools/misc/datetime_defs.hpp"
#include "../../cpptools/misc/ERROR.hpp"

#include <string>
#include <map>

using namespace std;

time_sec intervalToSecond(const string& interval) {
    // Using static const makes the map initialized only once.
    static const map<string, time_sec> intervalSeconds = {
        { "1s", 1 },
        { "1m", 60 },
        { "10m", 10 * 60 },
        { "15m", 15 * 60 },
        { "30m", 30 * 60 },
        { "1h", 60 * 60 },
        { "2h", 2 * 60 * 60 },
        { "4h", 4 * 60 * 60 },
        { "8h", 8 * 60 * 60 },
        { "12h", 12 * 60 * 60 },
        { "1d", 24 * 60 * 60 },
        { "1w", 7 * 24 * 60 * 60 },
    };

    try {
        return intervalSeconds.at(interval);
    } catch (const exception& e) {
        throw ERROR("Invalid interval string: '" + interval + "' " + e.what());
    }
}
