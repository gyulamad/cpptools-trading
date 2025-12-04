#pragma once

#include "Candle.hpp"
#include "AccountData.hpp"
#include "../misc/Logger.hpp"
// #include "../misc/EGA_COLORS.hpp"

#include <iostream>

using namespace std;

class Exchange {
public:
    Exchange(
        bool logsOnError, // TODO: initializer function?!
        bool showsOnError,
        bool throwsOnError
    ):
        logsOnError(logsOnError),
        showsOnError(showsOnError),
        throwsOnError(throwsOnError)
    {}

    virtual ~Exchange() {}

    // Calculates how much quoted asset (cash) we would have in the given moment if we sell all asset.
    // Should return the total balance (invested + remaining) in quoted asset. (The results may unrealized) 
    virtual float getBalanceTotal() = 0;

    // Invested quote asset, calculates the price of the asset we have in quoted asset.
    // Quoted value of asset amount. (The results may unrealized) 
    virtual float getBalanceUsed() = 0;

    // Depends on specific exchange but typically: free = total - used
    virtual float getBalanceFree() = 0;

    // Depends on specific exchange but typically: used percentage = used / total
    virtual float getBalanceUsedPc() = 0;

    // Depends on specific exchange but typically: free percentage = free / total
    virtual float getBalanceFreePc() = 0;

    // Calculates how much asset amount we would have in the given moment if buy as much as possible.
    // Should return the total value (invested + remaining) in asset amount. (The results may unrealized) 
    virtual float getAssetTotal() = 0;
    
    // Invested asset amount, calculates the value of the asset we have in asset amount.
    // Asset amount invested.
    virtual float getAssetUsed() = 0;

    // Depends on specific exchange but typically: free = total - used
    virtual float getAssetFree() = 0;

    // Depends on specific exchange but typically: used percentage = used / total
    virtual float getAssetUsedPc() = 0;

    // Depends on specific exchange but typically: free percentage = free / total
    virtual float getAssetFreePc() = 0;

    virtual AccountData getAccountData() {
        AccountData accountData(
            getBalanceTotal(),
            getBalanceUsed(),
            getAssetTotal(),
            getAssetUsed()
        );
        return accountData;
    }

    [[nodiscard]] bool buy(float quoted) {
        bool success = true;
        string label = "BUY quoted: " + to_string(quoted);
        try {
            if (!buyProtected(quoted)) 
                throw ERROR("Failed order");
        } catch (exception &e) {
            label += " (failed)";
            this->error(ERROR(label + EWHAT));
            success = false;
        }
        // charts->addLabel(
        //     charts->getPriceChart(), charts->getPriceCandleScale(),
        //     getTime(), getPrice(), label, EGA_LIGHT_RED
        // );
        return success;
    }

    [[nodiscard]] bool sell(float amount) {
        bool success = true;
        string label = "SELL amount: " + to_string(amount);
        try {
            if (!sellProtected(amount))
                throw ERROR("Failed order");
        } catch (exception &e) {
            label += " (failed)";
            this->error(ERROR(label + EWHAT));
            success = false;
        }
        // charts->addLabel(
        //     charts->getPriceChart(), charts->getPriceCandleScale(),
        //     getTime(), getPrice(), label, EGA_LIGHT_GREEN
        // );
        return success;
    }

    [[nodiscard]] bool buyLimit(float quoted, float limitPriced) {
        bool success = true;
        string label = "BUY quoted: " + to_string(quoted) + ", LIMIT: " + to_string(limitPriced);
        try {
            if (!buyLimitProtected(quoted, limitPriced))
                throw ERROR("Failed order");
        } catch (exception &e) {
            label += " (failed)";
            this->error(ERROR(label + EWHAT));
            success = false;
        }
        // charts->addLabel(
        //     charts->getPriceChart(), charts->getPriceCandleScale(),
        //     getTime(), getPrice(), label, EGA_LIGHT_RED
        // );
        return success;
    }

    [[nodiscard]] bool sellLimit(float amount, float limitPriced) {
        bool success = true;
        string label = "SELL amount: " + to_string(amount) + ", LIMIT: " + to_string(limitPriced);
        try {
            if (!sellLimitProtected(amount, limitPriced))
                throw ERROR("Failed order");
        } catch (exception &e) {
            label += " (failed)";
            this->error(ERROR(label + EWHAT));
            success = false;
        }
        // charts->addLabel(
        //     charts->getPriceChart(), charts->getPriceCandleScale(),
        //     getTime(), getPrice(), label, EGA_LIGHT_GREEN
        // );
        return success;
    }

    virtual size_t getPendingOrderCount() const = 0;
    virtual void cancelAllOrders() = 0;
    
protected:
    bool logsOnError = true;
    bool showsOnError = true;
    bool throwsOnError = true;

    [[nodiscard]] virtual bool buyProtected(float quoted) = 0;
    [[nodiscard]] virtual bool sellProtected(float amount) = 0;
    [[nodiscard]] virtual bool buyLimitProtected(float quoted, float limitPriced) = 0;
    [[nodiscard]] virtual bool sellLimitProtected(float amount, float limitPriced) = 0;
    
    virtual uint32_t getTime() = 0;
    virtual float getPrice() = 0;

    // virtual bool error(const string& errmsg) {
    virtual bool error(const runtime_error& e) {
        string errmsg = "Exchange error" + EWHAT;
        if (logsOnError)
            LOG_ERROR(errmsg); 
        if (showsOnError)
            cerr << ERROR(errmsg).what() << endl;           
        if (throwsOnError)
            throw ERROR(errmsg);
        return false;
    }
};
