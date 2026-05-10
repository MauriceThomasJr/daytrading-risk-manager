#ifndef ORDER_H
#define ORDER_H

#include "domain/TradeIntent.h"
#include <optional>

class Order {
public:
    // Factory: only way to construct an Order is from a validated intent.
    static Order fromValidatedIntent(const TradeIntent& intent, int size);

    Side getSide() const;
    const Instrument& getInstrument() const;
    double getEntryPrice() const;
    double getStopPrice() const;
    std::optional<double> getTargetPrice() const;
    int getSize() const;

private:
    Order(const TradeIntent& intent, int size);  // private constructor

    TradeIntent intent_;
    int size_;
};

#endif