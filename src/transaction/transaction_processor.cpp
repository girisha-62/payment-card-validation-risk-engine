#include "transaction/transaction_processor.hpp"

#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace {
std::string makeCardToken(
    const std::string& cardNumber,
    CardNetwork network) {

    // Educational tokenization substitute.
    // Never use std::hash as a production payment-token mechanism.
    std::size_t value =
        std::hash<std::string>{}(cardNumber);

    std::ostringstream out;
    out << "tok_"
        << CardValidator::networkName(network)
        << "_"
        << std::hex
        << value;

    return out.str();
}
}

TransactionProcessor::TransactionProcessor(
    CardValidator& validator,
    RiskEngine& riskEngine,
    TransactionStore& store,
    size_t threadCount)
    : validator_(validator),
      riskEngine_(riskEngine),
      store_(store),
      threadPool_(threadCount) {
}

void TransactionProcessor::process(
    const std::string& cardNumber,
    const std::string& expiry,
    const std::string& cvv,
    double amount,
    bool international) {

    // Use a promise so the menu waits for the worker result.
    auto done = std::make_shared<std::promise<void>>();
    auto future = done->get_future();

    threadPool_.enqueue(
        [this, cardNumber, expiry, cvv, amount, international, done] {
            try {
                processTransaction(
                    cardNumber,
                    expiry,
                    cvv,
                    amount,
                    international);
                done->set_value();
            } catch (...) {
                done->set_exception(std::current_exception());
            }
        });

    future.get();
}

void TransactionProcessor::processTransaction(
    std::string cardNumber,
    std::string expiry,
    std::string cvv,
    double amount,
    bool international) {

    std::cout << "\nProcessing transaction on worker thread...\n";

    if (amount <= 0.0) {
        std::cout << "Transaction declined: amount must be greater than zero.\n";
        return;
    }

    CardValidationResult validation =
        validator_.validate(cardNumber, expiry, cvv);

    std::cout << "\nCard Validation\n";
    std::cout << "Network      : "
              << CardValidator::networkName(validation.network) << "\n";
    std::cout << "Luhn valid   : "
              << (validation.luhnValid ? "YES" : "NO") << "\n";
    std::cout << "Expiry valid : "
              << (validation.expiryValid ? "YES" : "NO") << "\n";
    std::cout << "CVV valid    : "
              << (validation.cvvValid ? "YES" : "NO") << "\n";
    std::cout << "Card valid   : "
              << (validation.valid ? "YES" : "NO") << "\n";

    // Invalid cards are rejected before authorization.
    if (!validation.valid) {
        std::cout << "\nDecision: DECLINED\n";
        std::cout << "Reason: Card validation failed.\n";
        return;
    }

    Transaction transaction;

    transaction.transactionId =
        "TXN-" +
        std::to_string(transactionCounter_.fetch_add(1));

    transaction.cardToken =
        makeCardToken(cardNumber, validation.network);

    transaction.network =
        CardValidator::networkName(validation.network);

    transaction.amount = amount;
    transaction.international = international;
    transaction.cardValid = validation.valid;
    transaction.expiryMonth = std::stoi(expiry.substr(0, 2));
    transaction.expiryYear = 2000 + std::stoi(expiry.substr(2, 2));
    transaction.createdAt = std::chrono::system_clock::now();

    int recentTransactions =
    store_.countRecentTransactions(
        transaction.cardToken);

    RiskResult risk =
        riskEngine_.evaluate(
            transaction,
            recentTransactions);

    transaction.riskScore = risk.score;
    transaction.riskLevel =
        RiskEngine::levelName(risk.level);
    transaction.reason = risk.reason;
    transaction.status =
        risk.approved
        ? TransactionStatus::APPROVED
        : TransactionStatus::DECLINED;

    store_.add(transaction);

    std::cout << "\n============================================\n";
    std::cout << "        TRANSACTION RESULT\n";
    std::cout << "============================================\n";

    std::cout << "Transaction ID : "
              << transaction.transactionId << "\n";

    std::cout << "Network        : "
              << transaction.network << "\n";

    std::cout << "Amount         : "
              << std::fixed << std::setprecision(2)
              << transaction.amount << "\n";

    std::cout << "Risk Score     : "
              << transaction.riskScore << "\n";

    std::cout << "Risk Level     : "
              << transaction.riskLevel << "\n";

    std::cout << "Decision       : "
              << (risk.approved ? "APPROVED" : "DECLINED")
              << "\n";

    std::cout << "Reason         : "
              << transaction.reason << "\n";

    std::cout << "============================================\n";
}
