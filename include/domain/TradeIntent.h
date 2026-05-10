#ifndef TRADE_INTENT_H
#define TRADE_INTENT_H

#include "domain/Side.h"
#include "domain/Instrument.h"
#include <optional>

class TradeIntent {
public:
    TradeIntent(Side side,
                const Instrument& instrument,
                double entryPrice,
                double stopPrice,
                std::optional<double> targetPrice = std::nullopt);

    Side getSide() const;
    const Instrument& getInstrument() const;
    double getEntryPrice() const;
    double getStopPrice() const;
    std::optional<double> getTargetPrice() const;
private:
    Side side_;
    Instrument instrument_;
    double entryPrice_;
    double stopPrice_;
    std::optional<double> targetPrice_;
};
#endif