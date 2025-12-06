#pragma once

#include "HistoryArguments.hpp"
#include "Strategy.hpp"
#include "TestExchange.hpp"

#include "../misc/DynLoader.hpp"
#include "../misc/date_to_sec.hpp"
#include "../misc/get_time_sec.hpp"
#include "../misc/explode.hpp"
#include "../math/Calculation.hpp"

const string STRATEGIES_DIR = fix_path(__DIR__ + "/strategies") + "/";
const string EXCHANGES_DIR = fix_path(__DIR__ + "/exchanges") + "/";

class BacktestArguments: public HistoryArguments {
public:
    BacktestArguments(
        int argc, char* argv[], 
        DynLoader& loader
    ): 
        HistoryArguments(argc, argv, loader)
    {
        addHelp({ "chartfile", "c"}, "Chart file");
        addHelp({ "strategy", "s"}, "Strategy");
        addHelp({ "exchange", "e"}, "Exchange");
        addHelp({ "period-start", "p"}, "Period start");
        addHelp({ "period-end", "r"}, "Period end");

        chartfile = get<string>("chartfile"); //get<string>(1);

        strategyLib = STRATEGIES_DIR + get<string>("strategy") + LIB_EXT;
        strategy = loadStrategy();

        exchangeLib = EXCHANGES_DIR + get<string>("exchange") + LIB_EXT;
        exchange = loadExchange();

        periodStart = has("period-start") ? date_to_sec(get<string>("period-start")) : 0;
        periodEnd = has("period-end") ? date_to_sec(get<string>("period-end")) : get_time_sec();
    }

    virtual ~BacktestArguments() {}

    string getChartfile() const { return chartfile; }

    string getStrategyLib() const { return strategyLib; }
    Strategy* getStrategy() const { return strategy; }

    string getExchangeLib() const { return exchangeLib; }
    TestExchange* getExchange() const { return exchange; }

    Calculation<float>* getCalculation() const { return calculation; }

    time_sec getPeriodStart() const { return periodStart; }
    time_sec getPeriodEnd() const { return periodEnd; }

    vector<Candle> loadCandles() const {
        return HistoryArguments::loadCandles(periodStart, periodEnd);
    }
    
protected:

    Strategy* loadStrategy() {
        Strategy* strategy = 
            loader.load<Strategy>(strategyLib);
        return strategy;
    }

    TestExchange* loadExchange() {
        TestExchange* exchange = 
            loader.load<TestExchange>(exchangeLib);
        return exchange;
    }

    string chartfile;
    string strategyLib;
    string exchangeLib;
    time_sec periodStart;
    time_sec periodEnd;
    TestExchange* exchange = nullptr;
    Strategy* strategy = nullptr;
    vector<Candle> candles;
    Calculation<float>* calculation = nullptr;
};
