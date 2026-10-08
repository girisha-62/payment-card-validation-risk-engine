#ifndef TRANSACTION_HPP
#define TRANSACTION_HPP

#include <string>
#include <chrono>

enum class TransactionStatus {
    APPROVED,
    DECLINED
};

struct Transaction {

    // Transaction information
    std::string transactionId;

    // Card information
    std::string cardToken;
    std::string network;

    // Card expiry
    int expiryMonth = 0;
    int expiryYear = 0;

    // Validation
    bool cardValid = false;

    // Transaction details
    double amount = 0.0;
    bool international = false;

    // Risk information
    int riskScore = 0;
    std::string riskLevel;
    std::string reason;

    // Status
    TransactionStatus status =
        TransactionStatus::DECLINED;

    // Timestamp
    std::chrono::system_clock::time_point createdAt;
};

#endif