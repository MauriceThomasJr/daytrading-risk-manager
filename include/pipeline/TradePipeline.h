#ifndef TRADE_PIPELINE_H
#define TRADE_PIPELINE_H

#include "checklist/ChecklistGate.h"
#include "checklist/ChecklistTemplate.h"
#include "checklist/ChecklistResponse.h"
#include "domain/Account.h"
#include "domain/TradeIntent.h"
#include "risk/RiskManager.h"
#include "pipeline/TradeSubmissionResult.h"
#include "journal/ITradeJournal.h"
#include "broker/IBrokerAdapter.h"

class TradePipeline {
public:
    TradePipeline(ChecklistGate gate, RiskManager rules,
                  ITradeJournal& journal, IBrokerAdapter& broker);

    TradeSubmissionResult submit(const TradeIntent& intent,
                                 Account& account,
                                 const ChecklistTemplate& tmpl,
                                 const ChecklistResponse& responses) const;

private:
    ChecklistGate gate_;
    RiskManager rules_;
    ITradeJournal& journal_;
    IBrokerAdapter& broker_;
};

#endif