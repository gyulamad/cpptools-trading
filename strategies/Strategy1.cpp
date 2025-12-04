#include "../../misc/EXTERN.hpp"

#include "../Strategy.hpp"

class Strategy1: public Strategy {
public:
    Strategy1(): Strategy() {}
    virtual ~Strategy1() {}

    virtual void onStart(const Candle&) override {}

    virtual void onCandleClose(const Candle&) override {}
};

EXTERN(Strategy1, (), ());