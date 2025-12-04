#pragma once

#include "LimitOrder.hpp"
#include "Exchange.hpp"
#include "Strategy.hpp"

#include "../misc/Value.hpp"

class TestExchange: public Exchange {
public:
    TestExchange(
        bool logsOnError,
        bool showsOnError,
        bool throwsOnError
    ):
        Exchange(logsOnError, showsOnError, throwsOnError)
    {}

    virtual ~TestExchange() {}
    
    float getBalanceTotal() override {
        return balance + asset * price;
    }

    float getBalanceUsed() override {
        return asset * price;
    }

    float getBalanceFree() override {
        return getBalanceTotal() - getBalanceUsed();
    }

    float getBalanceUsedPc() override {
        return getBalanceUsed() / getBalanceTotal();
    }

    float getBalanceFreePc() override {
        return getBalanceFree() / getBalanceTotal();
    }
    
    float getAssetTotal() override {
        return asset + balance / price;
    }

    float getAssetUsed() override {
        return asset;
    }

    float getAssetFree() override {
        return getAssetTotal() - getAssetUsed();
    }

    float getAssetUsedPc() override {
        return getAssetUsed() / getAssetTotal();
    }

    float getAssetFreePc() override {
        return getAssetFree() / getAssetTotal();
    }
    
    // Get number of pending limit orders
    size_t getPendingOrderCount() const override {
        return limitOrders.size();
    }
    
    // Cancel all pending orders (return reserved funds/assets)
    void cancelAllOrders() override {
        for (const auto& order: limitOrders)
            if (order.type == OrderType::BUY_LIMIT) balance += order.amount; // Return reserved cash
            else asset += order.amount; // Return reserved assets
        limitOrders.clear();
    }


    // ============ Internal use only, DO NOT call in strategy! ============

    virtual void setTime(uint32_t time) { this->time = time; }
    virtual void setPrice(float price) { this->price = price; }
    
    void setBalance(float balance) { this->balance = balance; }
    void setAsset(float asset) { this->asset = asset; }
    float getBalance() const { return balance; }
    float getAsset() const { return asset; }
    void setFeeMakerBuyPc(float feeMakerBuyPc) { this->feeMakerBuyPc = feeMakerBuyPc; }
    void setFeeMakerSellPc(float feeMakerSellPc) { this->feeMakerSellPc = feeMakerSellPc; }
    void setFeeTakerBuyPc(float feeTakerBuyPc) { this->feeTakerBuyPc = feeTakerBuyPc; }
    void setFeeTakerSellPc(float feeTakerSellPc) { this->feeTakerSellPc = feeTakerSellPc; }

    // Updated processLimitOrders method with maker fees
    void processLimitOrders(const Candle& candle) {
        auto it = limitOrders.begin();
        
        while (it != limitOrders.end()) {
            bool orderFilled = false;
            
            if (it->type == OrderType::BUY_LIMIT) {
                // Buy order executes when market goes at or below limit price
                if (candle.getLow() <= it->price) {
                    // Execute buy order with maker fee
                    float fee = it->amount * feeMakerBuyPc; // Fee in quoted currency
                    float net = (it->amount - fee) / it->price; // Net asset amount after fee
                    
                    asset += net;
                    orderFilled = true;
                }
            } else { // SELL_LIMIT
                // Sell order executes when market goes at or above limit price
                if (candle.getHigh() >= it->price) {
                    // Execute sell order with maker fee
                    float gross = it->amount * it->price; // Gross proceeds
                    float fee = gross * feeMakerSellPc; // Fee on proceeds
                    float net = gross - fee; // Net amount we receive
                    
                    balance += net;
                    orderFilled = true;
                }
            }
            
            if (orderFilled) it = limitOrders.erase(it);
            else ++it;
        }
    }

protected:

    uint32_t time;
    float price = 0;

    float balance;
    float asset;    

    vector<LimitOrder> limitOrders;

    float feeTakerBuyPc, feeTakerSellPc, feeMakerBuyPc, feeMakerSellPc;

    virtual uint32_t getTime() override { return time; }
    virtual float getPrice() override { return price; }
    
    [[nodiscard]]
    bool buyProtected(float quoted) override {
        if (price <= .0f)
            return error(ERROR("Negative price: " + to_string(price)));

        if (quoted <= .0f) // TODO: pre-validation can be in a central place to prevent errors on live systems too?
            return error(ERROR("Negative quoted amount: " + to_string(quoted)));
        
        // Calculate total cost including taker fee
        if (Value(quoted) > Value(balance))
            return error(ERROR("Insufficient balance: " + to_string(quoted) + " > " + to_string(balance)));
        
        float amount = quoted / price; // Asset amount we get
        balance -= quoted; // Deduct quoted amount
        float fee = amount * feeTakerBuyPc;
        asset += amount - fee; // TODO: pre-calculation can be in a central place to valudate the backtesting on live systems?
        
        return true;
    }

    [[nodiscard]]
    bool sellProtected(float amount) override {
        if (price <= .0f)
            return error(ERROR("Negative price: " + to_string(price)));

        if (amount <= .0f)
            return error(ERROR("Negative amount: " + to_string(amount)));

        if (Value(amount) > Value(asset))
            return error(ERROR("Insufficient amount: " + to_string(amount) + " > " + to_string(asset)));
        
        asset -= amount;
        float quoted = amount * price; // Gross proceeds
        float fee = quoted * feeTakerSellPc; // Fee on proceeds
        balance += quoted - fee;
        
        return true;
    }

    // Place a buy limit order (will execute when market price <= limit price)
    [[nodiscard]]
    bool buyLimitProtected(float quoted, float limitPrice) override {
        if (limitPrice <= .0f)
            return error(ERROR("Negative limit price: " + to_string(price)));

        if (quoted <= .0f) // Invalid parameters
            return error(ERROR("Negative quoted amount: " + to_string(quoted)));

        if (Value(quoted) > Value(balance)) // Insufficient balance
            return error(ERROR("Insufficient balance: " + to_string(quoted) + " > " + to_string(balance)));
        
        balance -= quoted; // Reserve the cash for this order
        
        // Add to pending orders
        limitOrders.emplace_back(OrderType::BUY_LIMIT, limitPrice, quoted);
        
        return true;
    }
    
    // Place a sell limit order (will execute when market price >= limit price)
    [[nodiscard]]
    bool sellLimitProtected(float amount, float limitPrice) override {
        if (limitPrice <= .0f)
            return error(ERROR("Negative limit price: " + to_string(price)));

        if (amount <= .0f) // Invalid parameters
            return error(ERROR("Negative amount: " + to_string(amount)));

        if (Value(amount) > Value(asset)) // Insufficient assets
            return error(ERROR("Insufficient assets: " + to_string(amount) + " > " + to_string(asset)));
        
        // Reserve the assets for this order
        asset -= amount;
        
        // Add to pending orders
        limitOrders.emplace_back(OrderType::SELL_LIMIT, limitPrice, amount);
        
        return true;
    }
};


#ifdef TEST

#include "../misc/capture_cerr.hpp"
#include "../misc/ConsoleLogger.hpp"

class TestExchangeMock : public TestExchange {
public:
    TestExchangeMock(
        bool logsOnError = true,
        bool showsOnError = true,
        bool throwsOnError = true
    ): 
        TestExchange(logsOnError, showsOnError, throwsOnError)
    {}

    virtual ~TestExchangeMock() {}

    void setBalance(float balance) { this->balance = balance; }
    void setAsset(float asset) { this->asset = asset; }
    float getBalance() const { return balance; }
    float getAsset() const { return asset; }
    void setPrice(float price) { this->price = price; }
    void setFeeMakerBuyPc(float feeMakerBuyPc) { this->feeMakerBuyPc = feeMakerBuyPc; }
    void setFeeMakerSellPc(float feeMakerSellPc) { this->feeMakerSellPc = feeMakerSellPc; }
    void setFeeTakerBuyPc(float feeTakerBuyPc) { this->feeTakerBuyPc = feeTakerBuyPc; }
    void setFeeTakerSellPc(float feeTakerSellPc) { this->feeTakerSellPc = feeTakerSellPc; }
    void processLimitOrders(const Candle &candle) { TestExchange::processLimitOrders(candle); }
} exchange;

TEST(test_TestExchange_getBalanceTotal_calculates_portfolio_value) {
    TestExchangeMock exchange;
    exchange.setBalance(5000);
    exchange.setAsset(10);
    exchange.setPrice(100);
    
    float expected = 5000 + (10 * 100); // 15000
    assert(abs(exchange.getBalanceTotal() - expected) < 0.001f && "Balance total should be cash + asset value");
}

TEST(test_TestExchange_getBalanceUsed_calculates_asset_value) {
    TestExchangeMock exchange;
    exchange.setAsset(5);
    exchange.setPrice(200);
    
    float expected = 5 * 200; // 1000
    assert(abs(exchange.getBalanceUsed() - expected) < 0.001f && "Balance used should be asset * price");
}

TEST(test_TestExchange_getAssetTotal_calculates_potential_assets) {
    TestExchangeMock exchange;
    exchange.setBalance(1000);
    exchange.setAsset(2);
    exchange.setPrice(100);
    
    float expected = 2 + (1000 / 100); // 12
    assert(abs(exchange.getAssetTotal() - expected) < 0.001f && "Asset total should be current + potential from cash");
}

TEST(test_TestExchange_getAssetUsed_returns_current_assets) {
    TestExchangeMock exchange;
    exchange.setAsset(7.5);
    
    assert(abs(exchange.getAssetUsed() - 7.5) < 0.001f && "Asset used should return current asset amount");
}

TEST(test_TestExchange_buy_successful_purchase) {
    TestExchangeMock exchange;
    exchange.setBalance(1000);
    exchange.setAsset(0);
    exchange.setPrice(50);
    exchange.setFeeTakerBuyPc(0.01); // 1%
    
    bool result = exchange.buy(500); // Buy $500 worth
    
    assert(result && "Buy should succeed with sufficient balance");
    assert(abs(exchange.getBalance() - 500) < 0.001f && "Balance should be reduced by quoted"); // 1000 - 500
    assert(abs(exchange.getAsset() - 9.9) < 0.001f && "Asset should increase by quoted/price - fee"); // 500/50 - fee
}

TEST(test_TestExchange_buy_insufficient_balance) {
    TestExchangeMock exchange(false, true, false);
    exchange.setBalance(100);
    exchange.setAsset(0);
    exchange.setPrice(50);
    exchange.setFeeTakerBuyPc(0.01);
    
    bool result = true;
    capture_cerr([&result, &exchange]() {
        result = exchange.buy(500);
    });
    
    assert(!result && "Buy should fail with insufficient balance");
    assert(abs(exchange.getBalance() - 100) < 0.001f && "Balance should remain unchanged");
    assert(abs(exchange.getAsset() - 0) < 0.001f && "Asset should remain unchanged");
}

TEST(test_TestExchange_buy_invalid_parameters) {
    bool result1 = true;
    bool result2 = true;
    bool result3 = true;

    capture_cerr([&]() {
        TestExchangeMock exchange(false, true, false);
        exchange.setBalance(1000);
        exchange.setPrice(50);
        
        result1 = exchange.buy(-100);
        result2 = exchange.buy(0);

        exchange.setPrice(0);
        result3 = exchange.buy(100);
    });

    assert(!result1 && "Buy should reject negative amounts");
    assert(!result2 && "Buy should reject zero amounts");
    
    assert(!result3 && "Buy should reject zero price");
}

TEST(test_TestExchange_sell_successful_sale) {
    TestExchangeMock exchange;
    exchange.setBalance(0);
    exchange.setAsset(10);
    exchange.setPrice(100);
    exchange.setFeeTakerSellPc(0.02); // 2%
    
    bool result = exchange.sell(5); // Sell 5 assets
    
    assert(result && "Sell should succeed with sufficient assets");
    assert(abs(exchange.getAsset() - 5) < 0.001f && "Asset should decrease by sold amount");
    assert(abs(exchange.getBalance() - 490) < 0.001f && "Balance should increase by proceeds minus fee"); // 5*100 - 5*100*0.02
}

TEST(test_TestExchange_sell_insufficient_assets) {
    TestExchangeMock exchange(false, true, false);
    exchange.setBalance(0);
    exchange.setAsset(3);
    exchange.setPrice(100);
    
    bool result = true;
    capture_cerr([&result, &exchange]() {
        result = exchange.sell(5);
    });
    
    assert(!result && "Sell should fail with insufficient assets");
    assert(abs(exchange.getAsset() - 3) < 0.001f && "Asset should remain unchanged");
    assert(abs(exchange.getBalance() - 0) < 0.001f && "Balance should remain unchanged");
}

TEST(test_TestExchange_buyLimit_places_order) {
    TestExchangeMock exchange;
    exchange.setBalance(1000);
    exchange.setAsset(0);
    
    bool result = exchange.buyLimit(500, 90);
    
    assert(result && "Buy limit should succeed");
    assert(abs(exchange.getBalance() - 500) < 0.001f && "Balance should be reserved");
    assert(exchange.getPendingOrderCount() == 1 && "Should have one pending order");
}

TEST(test_TestExchange_sellLimit_places_order) {
    TestExchangeMock exchange;
    exchange.setBalance(0);
    exchange.setAsset(10);
    
    bool result = exchange.sellLimit(5, 110);
    
    assert(result && "Sell limit should succeed");
    assert(abs(exchange.getAsset() - 5) < 0.001f && "Assets should be reserved");
    assert(exchange.getPendingOrderCount() == 1 && "Should have one pending order");
}

TEST(test_TestExchange_buyLimit_insufficient_balance) {
    TestExchangeMock exchange(false, true, false);
    exchange.setBalance(100);
    
    bool result = true;
    capture_cerr([&result, &exchange]() {
        result = exchange.buyLimit(500, 90);
    });
    
    assert(!result && "Buy limit should fail with insufficient balance");
    assert(exchange.getPendingOrderCount() == 0 && "Should have no pending orders");
}

TEST(test_TestExchange_sellLimit_insufficient_assets) {
    TestExchangeMock exchange(false, true, false);
    exchange.setAsset(3);
    
    bool result = true;
    capture_cerr([&result, &exchange]() {
        result = exchange.sellLimit(5, 110);
    });
    
    assert(!result && "Sell limit should fail with insufficient assets");
    assert(exchange.getPendingOrderCount() == 0 && "Should have no pending orders");
}

TEST(test_TestExchange_cancelAllOrders_returns_reserved_funds) {
    TestExchangeMock exchange;
    exchange.setBalance(1000);
    exchange.setAsset(10);
    
    assert(exchange.buyLimit(300, 90));   // Reserve 300 cash
    assert(exchange.sellLimit(4, 110));   // Reserve 4 assets
    
    assert(exchange.getPendingOrderCount() == 2 && "Should have two pending orders");
    
    exchange.cancelAllOrders();
    
    assert(exchange.getPendingOrderCount() == 0 && "Should have no pending orders");
    assert(abs(exchange.getBalance() - 1000) < 0.001f && "Cash should be returned");
    assert(abs(exchange.getAsset() - 10) < 0.001f && "Assets should be returned");
}

TEST(test_TestExchange_processLimitOrders_executes_buy_when_price_drops) {
    TestExchangeMock exchange;
    exchange.setBalance(1000);
    exchange.setAsset(0);
    exchange.setPrice(100);
    exchange.setFeeMakerBuyPc(0.004); // 0.4%
    
    assert(exchange.buyLimit(500, 95)); // Limit buy at 95
    
    // Create candle that touches the limit price
    Candle candle;
    candle.setHigh(100);
    candle.setLow(94);  // Low touches limit price
    candle.setClose(96);
    
    exchange.processLimitOrders(candle);
    
    assert(exchange.getPendingOrderCount() == 0 && "Order should be filled");
    assert(exchange.getAsset() > 0 && "Should have acquired assets");
    // Net asset = (500 - 500*0.004) / 95 ≈ 5.242
    assert(abs(exchange.getAsset() - 5.242) < 0.01f && "Asset amount should account for fees");
}

TEST(test_TestExchange_processLimitOrders_executes_sell_when_price_rises) {
    TestExchangeMock exchange;
    exchange.setBalance(0);
    exchange.setAsset(10);
    exchange.setPrice(100);
    exchange.setFeeMakerSellPc(0.004); // 0.4%
    
    assert(exchange.sellLimit(5, 105)); // Limit sell at 105
    
    // Create candle that reaches the limit price
    Candle candle;
    candle.setHigh(106); // High reaches limit price
    candle.setLow(102);
    candle.setClose(104);
    
    exchange.processLimitOrders(candle);
    
    assert(exchange.getPendingOrderCount() == 0 && "Order should be filled");
    assert(exchange.getBalance() > 0 && "Should have received cash");
    // Net proceeds = 5*105 - 5*105*0.004 = 525 - 2.1 = 522.9
    assert(abs(exchange.getBalance() - 522.9) < 0.1f && "Balance should reflect proceeds minus fees");
}

TEST(test_TestExchange_processLimitOrders_does_not_execute_unfilled_orders) {
    TestExchangeMock exchange;
    exchange.setBalance(1000);
    exchange.setAsset(10);
    exchange.setFeeMakerBuyPc(0.0f);
    exchange.setFeeMakerSellPc(0.0f);
    
    assert(exchange.buyLimit(500, 90));   // Buy limit at 90
    assert(exchange.sellLimit(5, 110));   // Sell limit at 110
    
    // Create candle that doesn't reach either limit
    Candle candle;
    candle.setHigh(105);
    candle.setLow(95);
    candle.setClose(100);
    
    exchange.processLimitOrders(candle);
    
    assert(exchange.getPendingOrderCount() == 2 && "Orders should remain unfilled");
    assert(abs(exchange.getBalance() - 500) < 0.001f && "Cash should remain reserved");
    assert(abs(exchange.getAsset() - 5) < 0.001f && "Assets should remain reserved");
}

#endif
