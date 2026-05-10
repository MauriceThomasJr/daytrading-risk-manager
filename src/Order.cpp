#include "domain/Order.h"


Order::Order(const TradeIntent& intent, int size) 
    : intent_(intent), size_(size) {}

Order Order::fromValidatedIntent(const TradeIntent& intent, int size) {
    return Order(intent, size);
}

Side Order::getSide() const {
    return intent_.getSide();
}
const Instrument& Order::getInstrument() const {
    return intent_.getInstrument();
}
double Order::getEntryPrice() const {
    return intent_.getEntryPrice();

}
double Order::getStopPrice() const {
    return intent_.getStopPrice();
}
std::optional<double> Order::getTargetPrice() const {
    return intent_.getTargetPrice();
}
int Order::getSize() const {
    return size_;
}