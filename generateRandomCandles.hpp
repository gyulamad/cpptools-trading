#pragma once

#include <vector>

#include "Candle.hpp"

using namespace std;

// Function to generate random-like vector of Candle objects
vector<Candle> generateRandomCandles(
    int count,
    time_sec startTime = 0,
    int candleIntervalSec = 60,
    float startPrice = 100.0f,
    float maxDriftPercent = 0.4f,
    float minWickPercent = 0.0f,
    float maxWickPercent = 0.2f,
    float gapChancePercent = 1.0f,
    float gapSizePercent = 0.1f
) {
    vector<Candle> out;
    if (count <= 0) return out;

    out.reserve(count);

    // first candle
    Candle first;
    first.set(
        startTime,
        startPrice,
        startPrice,
        startPrice,
        startPrice,
        0.0f
    );
    out.push_back(first);

    for (int i = 1; i < count; i++) {
        const Candle &prev = out.back();
        Candle c;

        time_sec t = startTime + candleIntervalSec * i;

        // OPEN
        float open = prev.getClose();

        float r = (float)(rand() % 10000) / 100.0f; // 0..100%
        if (r < gapChancePercent) {
            float g = (float)(rand() % 2001 - 1000) / 1000.0f; // -1.0 .. +1.0
            open = open * (1.0f + g * (gapSizePercent / 100.0f));
        }

        // CLOSE
        float drift = (float)(rand() % (int)(maxDriftPercent * 2000 + 1)
                              - (int)(maxDriftPercent * 1000))
                              / 100000.0f;
        float close = open * (1.0f + drift);

        // WICKS
        float wickPercent =
            ((float)(rand() % (int)((maxWickPercent - minWickPercent) * 1000 + 1))
            / 1000.0f) + minWickPercent;
        wickPercent /= 100.0f;

        float high = max(open, close) * (1.0f + wickPercent);
        float low  = min(open, close) * (1.0f - wickPercent);

        // VOLUME (simple random)
        float volume = (float)(rand() % 1000 + 100);

        // now assign everything
        c.set(t, open, high, low, close, volume);

        out.push_back(c);
    }

    return out;
}
