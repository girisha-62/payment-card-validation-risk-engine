#include "risk/risk_engine.hpp"

RiskResult RiskEngine::evaluate(
    const Transaction& transaction,
    int recentTransactions) const {

    int score = 0;
    std::string reason;

    if (!transaction.cardValid) {
        score += 50;
        reason += "Invalid card. ";
    }

    if (transaction.amount > 100000.0) {
        score += 30;
        reason += "High transaction amount. ";
    }

    if (recentTransactions >= 5) {
        score += 20;
        reason += "High transaction velocity. ";
    }

    if (transaction.international) {
        score += 15;
        reason += "International transaction. ";
    }

    RiskLevel level;
    bool approved;

    if (score <= 30) {
        level = RiskLevel::LOW;
        approved = true;
    } else if (score <= 60) {
        level = RiskLevel::MEDIUM;
        approved = true;
    } else {
        level = RiskLevel::HIGH;
        approved = false;
    }

    if (reason.empty()) {
        reason = "Low risk transaction.";
    }

    return {score, level, approved, reason};
}

std::string RiskEngine::levelName(RiskLevel level) {
    switch (level) {
        case RiskLevel::LOW: return "LOW";
        case RiskLevel::MEDIUM: return "MEDIUM";
        case RiskLevel::HIGH: return "HIGH";
    }
    return "UNKNOWN";
}
