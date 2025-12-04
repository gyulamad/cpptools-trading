#pragma once

#include <string>
#include <vector>

#include "../misc/file_exists.hpp"
#include "../misc/mkdir.hpp"
#include "../misc/vector_load.hpp"
#include "../misc/vector_save.hpp"
#include "../misc/get_absolute_path.hpp"
#include "../math/linear_interpolation_search.hpp"
#include "../misc/array_slice.hpp"
#include "Candle.hpp"

using namespace std;

class CandleHistory {
public:
    CandleHistory() {}
    
    virtual ~CandleHistory() {}

    virtual string folder() = 0;

    string filename(const string& symbol, const string& interval) {
        string folder = ".data/" + this->folder() + "/candles";
        if (!file_exists(folder) && !mkdir(folder, true))
            throw ERROR("Unable to create folder: " + folder);
        return get_absolute_path(
            folder + "/" + symbol + "-" + interval + ".dat"
        );
    }
    
    vector<Candle> load(const string& symbol, const string& interval) {
        vector<Candle> candles;
        string file = filename(symbol, interval);
        // LOG_DEBUG("Load:" + file);
        if (file_exists(file)) vector_load<Candle>(candles, file);
        return candles;
    }

    vector<Candle> load(
        const string& symbol, const string& interval, time_sec from
    ) {
        vector<Candle> candles = load(symbol, interval);
        size_t first = linear_interpolation_search<Candle, time_sec>(
            candles, from, [](const Candle& candle) -> time_sec {
                return candle.getTime();
            }, true
        );
        return array_slice(candles, first);
    }

    vector<Candle> load(
        const string& symbol, const string& interval, 
        time_sec period_start, time_sec period_end
    ) {
        vector<Candle> candles = load(symbol, interval, period_start);

        size_t last = linear_interpolation_search<Candle, time_sec>(
            candles, period_end, [](const Candle& candle) -> time_sec {
                return candle.getTime();
            }, true
        );
        
        // Include all candles from start to last (inclusive)
        return array_slice(candles, 0, last + 1);
    }


    void save(
        const vector<Candle>& candles, 
        const string& symbol, 
        const string& interval
    ) {
        string file = filename(symbol, interval);
        vector_save<Candle>(candles, file);
    }

    virtual void update(const string& symbol, const string& interval) = 0;

    static vector<Candle> slice(
        const vector<Candle>& candles, 
        time_sec period_start, time_sec period_end
    ) {
        auto getter = [](const Candle& candle) -> time_sec {
            return candle.getTime();
        };

        size_t first = linear_interpolation_search<Candle, time_sec>(
            candles, period_start, getter, true
        );
        size_t last = linear_interpolation_search<Candle, time_sec>(
            candles, period_end, getter, true
        );

        // Return contiguous slice from first to last (inclusive)
        return array_slice(candles, first, (last - first) + 1);
    }
};


#ifdef TEST

#include "../misc/str_contains.hpp"

// Mock implementation for testing
class MockCandleHistory : public CandleHistory {
public:
    string folder() override {
        return "mock";
    }
    
    void update(const string& /*symbol*/, const string& /*interval*/) override {
        // Mock implementation
    }
    
    // Helper method to create test candles
    static vector<Candle> createTestCandles(time_sec start, time_sec end, time_sec interval) {
        vector<Candle> candles;
        for (time_sec t = start; t <= end; t += interval) {
            Candle c;
            c.setTime(t);
            candles.push_back(c);
        }
        return candles;
    }
};

// Test cases
TEST(test_CandleHistory_filename_creates_correct_path) {
    MockCandleHistory history;
    string result = history.filename("BTCUSDT", "1h");
    assert(str_contains(result, ".data/mock/candles/BTCUSDT-1h.dat") && 
           "Filename should include correct path and extension");
}

TEST(test_CandleHistory_load_returns_empty_vector_for_nonexistent_file) {
    MockCandleHistory history;
    vector<Candle> result = history.load("NONEXISTENT", "1m");
    assert(result.empty() && "Should return empty vector for nonexistent file");
}

TEST(test_CandleHistory_load_with_time_range_returns_correct_slice) {
    MockCandleHistory history;
    // Create test candles from timestamp 1000 to 2000, every 100 seconds
    vector<Candle> testCandles = MockCandleHistory::createTestCandles(1000, 2000, 100);
    
    // Save test data
    history.save(testCandles, "TEST", "1m");
    
    // Test loading with time range
    vector<Candle> result = history.load("TEST", "1m", 1200, 1800);
    
    // Verify we got the expected candles (1200, 1300, ..., 1800)
    assert(result.size() == 7 && "Should return 7 candles for range 1200-1800");
    assert(result.front().getTime() == 1200 && "First candle should be at 1200");
    assert(result.back().getTime() == 1800 && "Last candle should be at 1800");
}

TEST(test_CandleHistory_load_with_start_time_returns_all_after) {
    MockCandleHistory history;
    vector<Candle> testCandles = MockCandleHistory::createTestCandles(1000, 2000, 100);
    history.save(testCandles, "TEST", "1m");
    
    vector<Candle> result = history.load("TEST", "1m", 1500);
    
    assert(result.size() == 6 && "Should return 6 candles from 1500 onwards");
    assert(result.front().getTime() == 1500 && "First candle should be at 1500");
    assert(result.back().getTime() == 2000 && "Last candle should be at 2000");
}

TEST(test_CandleHistory_slice_returns_correct_subset) {
    vector<Candle> testCandles = MockCandleHistory::createTestCandles(1000, 2000, 100);
    vector<Candle> result = CandleHistory::slice(testCandles, 1300, 1700);
    
    assert(result.size() == 5 && "Should return 5 candles for range 1300-1700");
    assert(result.front().getTime() == 1300 && "First candle should be at 1300");
    assert(result.back().getTime() == 1700 && "Last candle should be at 1700");
}

TEST(test_CandleHistory_slice_handles_out_of_range) {
    vector<Candle> testCandles = MockCandleHistory::createTestCandles(1000, 2000, 100);
    vector<Candle> result = CandleHistory::slice(testCandles, 500, 2500);
    
    assert(result.size() == testCandles.size() && 
           "Should return all candles when range exceeds data");
    assert(result.front().getTime() == 1000 && 
           "First candle should be first available when start is before data");
    assert(result.back().getTime() == 2000 && 
           "Last candle should be last available when end is after data");
}

TEST(test_CandleHistory_save_and_load_roundtrip) {
    MockCandleHistory history;
    vector<Candle> testCandles = MockCandleHistory::createTestCandles(1000, 2000, 100);
    
    // Save and then load
    history.save(testCandles, "ROUNDTRIP", "1m");
    vector<Candle> result = history.load("ROUNDTRIP", "1m");
    
    assert(result.size() == testCandles.size() && 
           "Loaded data should match saved data size");
    for (size_t i = 0; i < testCandles.size(); i++) {
        assert(result[i].getTime() == testCandles[i].getTime() && 
               "Loaded candle times should match saved candle times");
    }
}

#endif
