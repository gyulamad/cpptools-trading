#pragma once

#include "Candle.hpp"
#include "Exchange.hpp"

using namespace std;

class Strategy {
public:

    Strategy() {}
    
    virtual ~Strategy() {}

    void setExchange(Exchange* exchange) {
        this->exchange = exchange;
    }

    // Called when the strategy starts.
    // Using the previously closed candle stick.
    virtual void onStart(const Candle&) = 0;

    // Called when a candle sick closes
    virtual void onCandleClose(const Candle&) = 0; 

protected:
    Exchange* exchange = nullptr;
};
