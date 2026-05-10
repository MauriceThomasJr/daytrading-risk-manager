#include "pipeline/TradePipeline.h"

TradePipeline::TradePipeline(ChecklistGate gate, RiskManager rules, ITradeJournal& journal)
    : gate_(gate), rules_(rules), journal_(journal) {}

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

    // All gates passed — build the order and record it
    result.accepted = true;
    result.order = Order::fromValidatedIntent(intent, size);
    journal_.record(*result.order);
    return result;
}