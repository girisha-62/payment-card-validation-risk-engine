#pragma once

#include "card/card_validator.hpp"
#include "risk/risk_engine.hpp"
#include "store/transaction_store.hpp"
#include "thread/thread_pool.hpp"

#include <atomic>
#include <future>
#include <string>

class TransactionProcessor {
public:
    TransactionProcessor(
        CardValidator& validator,
        RiskEngine& riskEngine,
        TransactionStore& store,
        size_t threadCount);

    void process(
        const std::string& cardNumber,
        const std::string& expiry,
        const std::string& cvv,
        double amount,
        bool international);

private:
    CardValidator& validator_;
    RiskEngine& riskEngine_;
    TransactionStore& store_;
    ThreadPool threadPool_;
    std::atomic<int> transactionCounter_{1001};

    void processTransaction(
        std::string cardNumber,
        std::string expiry,
        std::string cvv,
        double amount,
        bool international);
};
