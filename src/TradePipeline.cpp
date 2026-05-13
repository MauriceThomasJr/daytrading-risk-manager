#include "pipeline/TradePipeline.h"

TradePipeline::TradePipeline(ChecklistGate gate, RiskManager rules,
                             ITradeJournal& journal, IBrokerAdapter& broker)
    : gate_(gate), rules_(rules), journal_(journal), broker_(broker) {}

TradeSubmissionResult TradePipeline::submit(const TradeIntent& intent,
                                            Account& account,
                                            const ChecklistTemplate& tmpl,
                                            const ChecklistResponse& responses) const {
    TradeSubmissionResult result;
    result.accepted = false;

    // Gate 1: Checklist
    auto checklistEval = gate_.evaluate(tmpl, responses);
    if (!checklistEval.passed) {
        result.rejectionReasons = checklistEval.failureReasons;
        return result;
    }

    // Gate 2: Risk rules
    if (!rules_.canTakeNewTrade(account)) {
        result.rejectionReasons.push_back("Risk rules block new trades");
        return result;
    }

    // Sizing
    int size = rules_.calculatePositionSize(
        account,
        intent.getInstrument(),
        intent.getEntryPrice(),
        intent.getStopPrice());

    if (size <= 0) {
        result.rejectionReasons.push_back("Position size would be zero");
        return result;
    }

    // Build the order
    Order order = Order::fromValidatedIntent(intent, size);

    // Send to broker first — if it rejects, we don't journal a non-event
    BrokerResponse brokerResp = broker_.send(order);
    if (!brokerResp.accepted) {
        result.rejectionReasons.push_back(
            "Broker rejected: " + brokerResp.rejectionReason.value_or("unknown"));
        return result;
    }

    // Broker accepted — record it
    result.accepted = true;
    result.order = order;
    account.recordTradeOpened();    // ← count it as an open position
    journal_.record(order);
    return result;
}