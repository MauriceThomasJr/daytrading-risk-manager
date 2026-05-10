#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <string>

class Instrument {
public:
    Instrument(std::string symbol, double dollarPerPoint, double tickSize);
    const std::string& getSymbol() const;
    double getDollarPerPoint() const;
    double getTickSize() const;

private:
    std::string symbol_;
    double dollarPerPoint_;
    double tickSize_;
};
#endif 