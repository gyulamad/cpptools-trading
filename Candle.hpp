#pragma once

#include "../../cpptools/misc/datetime_defs.hpp"
#include "../../cpptools/misc/sec_to_datetime.hpp"

#include <iostream>

using namespace std;

class Candle {
public:
    Candle(
        time_sec time = 0,
        float open = .0f, 
        float high = .0f, 
        float low = .0f, 
        float close = .0f,
        float volume = .0f
    ):
        time(time),
        open(open),
        high(high),
        low(low),
        close(close),
        volume(volume)
    {}
    
    ~Candle() = default;

    void set(
        time_sec time = 0,
        float open = .0f, 
        float high = .0f, 
        float low = .0f, 
        float close = .0f,
        float volume = .0f
    ) {
        this->time = time;
        this->open = open;
        this->high = high;
        this->low = low;
        this->close = close;
        this->volume = volume;
    }

    time_sec getTime() const { return time; }
    float getOpen() const { return open; }
    float getHigh() const { return high; }
    float getLow() const { return low; }
    float getClose() const { return close; }
    float getVolume() const { return volume; }

    void setTime(time_sec time) {this->time = time; }
    void setOpen(float open) { this->open = open; }
    void setHigh(float high) { this->high = high; }
    void setLow(float low) { this->low = low; }
    void setClose(float close) { this->close = close; }
    void setVolume(float volume) { this->volume = volume; }

    string dump(bool show = false) const {
        string output = sec_to_datetime(time) + " " + to_string(time)
            + " O:" + to_string(open)
            + " H:" + to_string(high)
            + " L:" + to_string(low)
            + " C:" + to_string(close)
            + " V:" + to_string(volume);
        if (show) cout << output << endl;
        return output;
    }

private:
    time_sec time;
    float open;
    float high;
    float low;
    float close;
    float volume;
};
