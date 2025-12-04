
// DEPENDENCY: curl

#include <stdlib.h>                           // for atof, atoll
#include <iostream>                           // for basic_ostream, cout, endl
#include <map>                                // for map
#include <string>                             // for operator+, allocator
#include <utility>                            // for pair
#include <vector>                             // for vector
#include "../../misc/Curl.hpp"               // for Curl
#include "../../misc/ERROR.hpp"              // for ERROR
#include "../../misc/EXTERN.hpp"             // for EXTERN_DEFAULT
#include "../../misc/Logger.hpp"     // for createLogger
#include "../../misc/safe.hpp"       // for safe
#include "../../misc/datetime_to_sec.hpp"    // for datetime_to_sec
#include "../../misc/execute.hpp"            // for execute
#include "../../misc/explode.hpp"            // for explode
#include "../../misc/file_exists.hpp"        // for file_exists
#include "../../misc/file_get_contents.hpp"  // for file_get_contents
#include "../../misc/file_put_contents.hpp"  // for file_put_contents
#include "../../misc/mkdir.hpp"              // for mkdir
#include "../../misc/remove.hpp"             // for remove
#include "../../misc/replace_extension.hpp"  // for replace_extension
#include "../../misc/str_contains.hpp"       // for str_contains
#include "../../misc/str_replace.hpp"        // for str_replace
#include "../../misc/trim.hpp"               // for trim
#include "../../misc/ConsoleLogger.hpp"
#include "../Candle.hpp"                      // for Candle
#include "../CandleHistory.hpp"               // for CandleHistory

using namespace std;

class BinanceSpotCandleHistory: public CandleHistory {
public:
    BinanceSpotCandleHistory(): CandleHistory() {
        createLogger<ConsoleLogger>();
    }

    virtual ~BinanceSpotCandleHistory() {}

    string folder() override {
        return "binance/spot";
    }

    void update(const string& symbol, const string& interval) override {
        string url = "https://s3-ap-northeast-1.amazonaws.com/data.binance.vision?delimiter=/&prefix=data/spot/daily/klines/" + symbol + "/" + interval + "/";
        Curl curl;
        string result;
        string marker;
        vector<Candle> candles = load(symbol, interval);
        Curl::StreamCallback cb = [&result](const string& chunk) {
            result += chunk;
        };
        while (true) {
            result = "";
            if (!curl.GET(url + "&marker=" + marker, cb))
                throw ERROR("Unable to GET: " + url + "&marker=" + marker);
            const string start_mark = "<Contents><Key>";
            const string end_mark = ".CHECKSUM</Key>";
            vector<string> splits = explode(start_mark, result);        
            for (const string& split: splits) {
                if (!str_contains(split, end_mark)) continue;
                string zip = explode(end_mark, split)[0];
                string link = "https://data.binance.vision/" + zip;
                string date = str_replace({
                    { "data/spot/daily/klines/" + symbol + "/" + interval + "/" + symbol + "-" + interval + "-", "" },
                    { ".zip", ""}
                }, zip);
                if (candles.empty() || datetime_to_sec(date) > candles[candles.size() - 1].getTime()) {
                    cout << link << endl;
                    // download zip file
                    string result_safe = result;
                    result = "";
                    if (!curl.GET(link, cb))
                        throw ERROR("Unable to GET: " + link);
                    const string tempdir = ".data/binance/temp";
                    if (!file_exists(tempdir) && !mkdir(tempdir, true))
                        throw ERROR("Unable to create folder: " + tempdir);
                    const string zipf = tempdir + "/" + symbol + "-" + interval + "-" + date + ".zip";
                    file_put_contents(zipf, result, false, true);
                    execute("unzip -o " + zipf + " -d " + tempdir, true);
                    remove(zipf, true);
                    // parse csv
                    string csvf = replace_extension(zipf, ".csv");
                    string csv = file_get_contents(csvf);
                    vector<string> lines = explode("\n", csv);
                    for (const string& line: lines) {
                        if (trim(line).empty()) continue;
                        vector<string> cols = explode(",", trim(line));
                        Candle candle;
                        candle.setTime(atoll(cols[0].substr(0, 10).c_str()));
                        candle.setOpen(atof(cols[1].c_str()));
                        candle.setHigh(atof(cols[2].c_str()));
                        candle.setLow(atof(cols[3].c_str()));
                        candle.setClose(atof(cols[4].c_str()));
                        candle.setVolume(atof(cols[5].c_str()));
                        candles.push_back(candle);
                    }
                    remove(csvf, true);
                    result = result_safe;
                }
            }
            if (!str_contains(result, "<IsTruncated>true</IsTruncated>")) break;
            splits = explode("<NextMarker>", result);
            if (splits.empty()) break;
            splits = explode("</NextMarker>", splits[1]);
            if (splits.empty()) break;
            marker = splits[0];
        }
        save(candles, symbol, interval);
    }
};

EXTERN(BinanceSpotCandleHistory, (), ());
