#ifndef TRADE_PIPELINE_H
#define TRADE_PIPELINE_H

#include "checklist/ChecklistGate.h"
#include "checklist/ChecklistTemplate.h"
#include "checklist/ChecklistResponse.h"
#include "domain/Account.h"
#include "domain/TradeIntent.h"
#include "risk/RiskManager.h"
#include "pipeline/TradeSubmissionResult.h"

class TradePipeline {
public:
    TradePipeline(ChecklistGate gate, RiskManager rules);

    TradeSubmissionResult submit(const TradeIntent& intent,
                                 Account& account,
                                 const ChecklistTemplate& tmpl,
                                 const ChecklistResponse& responses) const;
private:
    ChecklistGate gate_;
    RiskManager rules_;
};

#endif