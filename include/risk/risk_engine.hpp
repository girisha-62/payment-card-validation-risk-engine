#pragma once

#include "transaction/transaction.hpp"
#include <string>

enum class RiskLevel {
    LOW,
    MEDIUM,
    HIGH
};

struct RiskResult {
    int score{0};
    RiskLevel level{RiskLevel::LOW};
    bool approved{true};
    std::string reason;
};

class RiskEngine {
public:
    RiskResult evaluate(
        const Transaction& transaction,
        int recentTransactions) const;

    static std::string levelName(RiskLevel level);
};
