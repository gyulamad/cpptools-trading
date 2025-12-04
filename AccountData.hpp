#pragma once

struct AccountData {
    float balanceTotal;
    float balanceUsed;
    float assetTotal;
    float assetUsed;
    AccountData(
        float balanceTotal,
        float balanceUsed,
        float assetTotal,
        float assetUsed
    ):
        balanceTotal(balanceTotal),
        balanceUsed(balanceUsed),
        assetTotal(assetTotal),
        assetUsed(assetUsed)
    {}

    virtual ~AccountData() {}
};
