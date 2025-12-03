#pragma once

#include "../misc/SetupArguments.hpp"
#include "../misc/DynLoader.hpp"
#include "CandleHistory.hpp"

#define HISTORIES_DIR __DIR__ + "/histories/"
#define HISTORY_LIB_EXT ".so"

class HistoryArguments: public Arguments {
public:
    HistoryArguments(int argc, char* argv[], DynLoader& loader): 
        Arguments(argc, argv), loader(loader)
    {
        historyLib = HISTORIES_DIR + get<string>("history") + HISTORY_LIB_EXT;
        history = loader.load<CandleHistory>(historyLib);

        symbol = get<string>("symbol");
        interval = get<string>("interval");
    }

    virtual ~HistoryArguments() {}

    string getSymbol() const { return symbol; }
    string getInterval() const { return interval; }

    string getHistoryLib() const { return historyLib; }
    CandleHistory* getHistory() const { return history; }


    vector<Candle> loadCandles(time_sec first, time_sec last) const {
        CandleHistory* history = getHistory();
        string symbol = getSymbol();
        string interval = getInterval();
        vector<Candle> candles = history->load(
            symbol, interval, 
            first, last
        );
        return candles;
    }

protected:

    DynLoader& loader;

    // string userSetupExt() const {
    //     return has("setup") ? "." + get<string>("setup") : "";
    // }

    string setupExt() const {
        return "." + symbol + "." + interval; // + userSetupExt();
    }

private:
    string historyLib;
    CandleHistory* history = nullptr;

    string symbol;
    string interval;
};
//