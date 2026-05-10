#include "domain/Instrument.h"


Instrument::Instrument(std::string symbol, double dollarPerPoint, double tickSize){
    symbol_ = symbol;
    dollarPerPoint_ = dollarPerPoint;
    tickSize_ = tickSize;
}

const std::string& Instrument::getSymbol() const{
    return symbol_;
}

double Instrument::getDollarPerPoint() const{
    return dollarPerPoint_;
}

double Instrument::getTickSize() const{
    return tickSize_;
}
