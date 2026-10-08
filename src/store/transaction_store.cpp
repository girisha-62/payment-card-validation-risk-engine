#include "store/transaction_store.hpp"

#include <iostream>
#include <string>


TransactionStore::TransactionStore(
    const std::string& connectionString
) {

    connection_ =
        PQconnectdb(connectionString.c_str());

    if (PQstatus(connection_) != CONNECTION_OK) {

        lastError_ =
            PQerrorMessage(connection_);

        std::cerr
            << "PostgreSQL connection failed: "
            << lastError_
            << "\n";
    }
    else {

        std::cout
            << "PostgreSQL connection successful!\n";
    }
}


TransactionStore::~TransactionStore() {

    if (connection_ != nullptr) {

        PQfinish(connection_);

        connection_ = nullptr;
    }
}


bool TransactionStore::isConnected() const {

    return connection_ != nullptr &&
           PQstatus(connection_) == CONNECTION_OK;
}


std::string TransactionStore::getLastError() const {

    return lastError_;
}


bool TransactionStore::add(
    const Transaction& transaction
) {

    if (!isConnected()) {

        return false;
    }


    /*
     * Step 1:
     * Make sure the card exists.
     *
     * transactions.card_token has a foreign key
     * referencing cards.card_token.
     */

    std::string expiryMonth =
        std::to_string(transaction.expiryMonth);

    std::string expiryYear =
        std::to_string(transaction.expiryYear);


    const char* cardValues[4];

    cardValues[0] =
        transaction.cardToken.c_str();

    cardValues[1] =
        transaction.network.c_str();

    cardValues[2] =
        expiryMonth.c_str();

    cardValues[3] =
        expiryYear.c_str();


    PGresult* cardResult =
        PQexecParams(

            connection_,

            "INSERT INTO cards "
            "(card_token, network, expiry_month, expiry_year) "
            "VALUES ($1, $2, $3, $4) "
            "ON CONFLICT (card_token) DO NOTHING",

            4,

            nullptr,

            cardValues,

            nullptr,

            nullptr,

            0
        );


    if (PQresultStatus(cardResult)
        != PGRES_COMMAND_OK) {

        lastError_ =
            PQerrorMessage(connection_);

        std::cerr
            << "Failed to save card: "
            << lastError_
            << "\n";

        PQclear(cardResult);

        return false;
    }


    PQclear(cardResult);


    /*
     * Step 2:
     * Insert the transaction.
     */

    std::string amount =
        std::to_string(transaction.amount);

    std::string international =
        transaction.international
        ? "true"
        : "false";

    std::string riskScore =
        std::to_string(transaction.riskScore);

    std::string status =
        transaction.status ==
        TransactionStatus::APPROVED
        ? "APPROVED"
        : "DECLINED";


    const char* transactionValues[9];

    transactionValues[0] =
        transaction.transactionId.c_str();

    transactionValues[1] =
        transaction.cardToken.c_str();

    transactionValues[2] =
        transaction.network.c_str();

    transactionValues[3] =
        amount.c_str();

    transactionValues[4] =
        international.c_str();

    transactionValues[5] =
        riskScore.c_str();

    transactionValues[6] =
        transaction.riskLevel.c_str();

    transactionValues[7] =
        status.c_str();

    transactionValues[8] =
        transaction.reason.c_str();


    PGresult* result =
        PQexecParams(

            connection_,

            "INSERT INTO transactions "
            "(transaction_id, card_token, network, "
            "amount, international, risk_score, "
            "risk_level, decision, reason) "
            "VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9) "
            "ON CONFLICT (transaction_id) DO NOTHING",

            9,

            nullptr,

            transactionValues,

            nullptr,

            nullptr,

            0
        );


    if (PQresultStatus(result)
        != PGRES_COMMAND_OK) {

        lastError_ =
            PQerrorMessage(connection_);

        std::cerr
            << "Failed to save transaction: "
            << lastError_
            << "\n";

        PQclear(result);

        return false;
    }


    PQclear(result);

    return true;
}


std::vector<Transaction>
TransactionStore::getAll() const {

    std::vector<Transaction> transactions;


    if (!isConnected()) {

        return transactions;
    }


    PGresult* result =
        PQexec(

            connection_,

            "SELECT transaction_id, "
            "card_token, network, amount, "
            "international, risk_score, "
            "risk_level, decision, reason "
            "FROM transactions "
            "ORDER BY created_at DESC"
        );


    if (PQresultStatus(result)
        != PGRES_TUPLES_OK) {

        PQclear(result);

        return transactions;
    }


    int rows =
        PQntuples(result);


    for (int i = 0; i < rows; i++) {

        Transaction transaction;


        transaction.transactionId =
            PQgetvalue(result, i, 0);


        transaction.cardToken =
            PQgetvalue(result, i, 1);


        transaction.network =
            PQgetvalue(result, i, 2);


        transaction.amount =
            std::stod(
                PQgetvalue(result, i, 3)
            );


        transaction.international =
            std::string(
                PQgetvalue(result, i, 4)
            ) == "t";


        transaction.riskScore =
            std::stoi(
                PQgetvalue(result, i, 5)
            );


        transaction.riskLevel =
            PQgetvalue(result, i, 6);


        std::string status =
            PQgetvalue(result, i, 7);


        if (status == "APPROVED") {

            transaction.status =
                TransactionStatus::APPROVED;
        }
        else {

            transaction.status =
                TransactionStatus::DECLINED;
        }


        transaction.reason =
            PQgetvalue(result, i, 8);


        transactions.push_back(
            transaction
        );
    }


    PQclear(result);

    return transactions;
}


int TransactionStore::countRecentTransactions(
    const std::string& cardToken
) const {

    if (!isConnected()) {

        return 0;
    }


    const char* values[1];

    values[0] =
        cardToken.c_str();


    PGresult* result =
        PQexecParams(

            connection_,

            "SELECT COUNT(*) "
            "FROM transactions "
            "WHERE card_token = $1 "
            "AND created_at >= "
            "CURRENT_TIMESTAMP - "
            "INTERVAL '5 minutes'",

            1,

            nullptr,

            values,

            nullptr,

            nullptr,

            0
        );


    if (PQresultStatus(result)
        != PGRES_TUPLES_OK) {

        PQclear(result);

        return 0;
    }


    int count =
        std::stoi(
            PQgetvalue(result, 0, 0)
        );


    PQclear(result);

    return count;
}