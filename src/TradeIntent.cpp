#include "domain/TradeIntent.h"


TradeIntent::TradeIntent(Side side,
                         const Instrument& instrument,
                         double entryPrice,
                         double stopPrice,
                         std::optional<double> targetPrice)
    : side_(side),
      instrument_(instrument),
      entryPrice_(entryPrice),
      stopPrice_(stopPrice),
      targetPrice_(targetPrice) {}

Side TradeIntent::getSide() const {
    return side_;
}
const Instrument& TradeIntent::getInstrument() const {
    return instrument_;
}

double TradeIntent::getEntryPrice() const {
    return entryPrice_;
}
double TradeIntent::getStopPrice() const {
    return stopPrice_;
}
std::optional<double> TradeIntent::getTargetPrice() const {
    return targetPrice_;
}