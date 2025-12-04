#include "../../misc/EXTERN.hpp"

#include "../Strategy.hpp"

class Strategy1: public Strategy {
public:
    Strategy1(): Strategy() {}
    virtual ~Strategy1() {}

    virtual void onStart(const Candle&) override {}

    virtual void onCandleClose(const Candle& candle) override {
        const int TBUY = 10;
        const int TSELL = 20;
        const int TREPEAT = 30;
        const float BUYPC = 20;
        const float SELLPC = 2;
        float price = candle.getClose();        
        i++;
        if (i == TBUY) {
            buyPrice = price;
            // if (exchange->getBalanceFreePc() > 0.05)
                if (!exchange->buy(exchange->getBalanceFree() / BUYPC)) {
                    LOG("BUY error");
                }
        }
        if (i == TSELL) {
            // if (price < buyPrice)
                if (!exchange->sell(exchange->getAssetUsed() / SELLPC)) {
                    LOG("SELL error");
                }
        }
        if (i == TREPEAT) i = 0;
    }

    int buyPrice = 0;
    int i = 0;
};

EXTERN(Strategy1, (), ());