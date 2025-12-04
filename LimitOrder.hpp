#pragma once

#include "OrderType.hpp"

struct LimitOrder {
    OrderType type;
    float price;
    float amount;  // For buy: cash amount, For sell: asset amount
    
    LimitOrder(OrderType type, float price, float amount):
        type(type), price(price), amount(amount) {}
};
