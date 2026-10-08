#ifndef TRANSACTION_STORE_HPP
#define TRANSACTION_STORE_HPP

#include "transaction/transaction.hpp"

#include <string>
#include <vector>
#include <libpq-fe.h>

class TransactionStore {

private:

    PGconn* connection_;

    std::string lastError_;

public:

    explicit TransactionStore(
        const std::string& connectionString
    );

    ~TransactionStore();

    bool isConnected() const;

    std::string getLastError() const;

    bool add(
        const Transaction& transaction
    );

    std::vector<Transaction> getAll() const;

    int countRecentTransactions(
        const std::string& cardToken
    ) const;
};

#endif