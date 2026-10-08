#include "card/card_validator.hpp"
#include "risk/risk_engine.hpp"
#include "store/transaction_store.hpp"
#include "transaction/transaction_processor.hpp"

#include <iostream>
#include <string>
#include <vector>


void showTransactionHistory(
    const TransactionStore& store
) {

    std::vector<Transaction> transactions =
        store.getAll();


    std::cout
        << "\n============================================\n";

    std::cout
        << "          TRANSACTION HISTORY\n";

    std::cout
        << "============================================\n";


    if (transactions.empty()) {

        std::cout
            << "No transactions found.\n";

        return;
    }


    for (const auto& transaction : transactions) {

        std::cout << "\n";


        std::cout
            << "Transaction ID : "
            << transaction.transactionId
            << "\n";


        std::cout
            << "Card Network   : "
            << transaction.network
            << "\n";


        std::cout
            << "Amount         : ₹"
            << transaction.amount
            << "\n";


        std::cout
            << "International  : "
            << (transaction.international
                ? "YES"
                : "NO")
            << "\n";


        std::cout
            << "Risk Score     : "
            << transaction.riskScore
            << "\n";


        std::cout
            << "Risk Level     : "
            << transaction.riskLevel
            << "\n";


        std::cout
            << "Decision       : "
            << (transaction.status ==
                TransactionStatus::APPROVED
                ? "APPROVED"
                : "DECLINED")
            << "\n";


        std::cout
            << "Reason         : "
            << transaction.reason
            << "\n";


        std::cout
            << "--------------------------------------------\n";
    }
}


int main() {

    // ========================================================
    // Create payment system components
    // ========================================================

    CardValidator validator;

    RiskEngine riskEngine;


    // PostgreSQL connection
    TransactionStore store(
        "host=localhost "
        "port=5432 "
        "dbname=payment_risk_db "
        "user=postgres "
        "password=postgres"
    );


    if (!store.isConnected()) {

        std::cerr
            << "\nUnable to connect to PostgreSQL.\n";

        std::cerr
            << "Error: "
            << store.getLastError()
            << "\n";

        return 1;
    }


    // ========================================================
    // Create transaction processor
    // 4 worker threads
    // ========================================================

    TransactionProcessor processor(
        validator,
        riskEngine,
        store,
        4
    );


    int choice;


    // ========================================================
    // Main menu
    // ========================================================

    std::cout
        << "\n============================================\n";

    std::cout
        << "     PAYMENT TRANSACTION SYSTEM\n";

    std::cout
        << "============================================\n";


    while (true) {

        std::cout << "\n";

        std::cout
            << "1. Process Transaction\n";

        std::cout
            << "2. View Transaction History\n";

        std::cout
            << "3. Exit\n";


        std::cout
            << "\nEnter your choice: ";


        if (!(std::cin >> choice)) {

            std::cin.clear();

            std::cin.ignore(
                10000,
                '\n'
            );

            std::cout
                << "\nInvalid input. Enter 1, 2 or 3.\n";

            continue;
        }


        switch (choice) {

            // =================================================
            // Process Transaction
            // =================================================

            case 1:
            {
                std::string cardNumber;

                std::string expiry;

                std::string cvv;

                double amount;

                char internationalInput;


                std::cout
                    << "\nEnter card number: ";

                std::cin
                    >> cardNumber;


                std::cout
                    << "Enter expiry (MMYY): ";

                std::cin
                    >> expiry;


                std::cout
                    << "Enter CVV: ";

                std::cin
                    >> cvv;


                std::cout
                    << "Enter transaction amount: ";

                std::cin
                    >> amount;


                std::cout
                    << "International transaction? (y/n): ";

                std::cin
                    >> internationalInput;


                bool international =
                    internationalInput == 'y' ||
                    internationalInput == 'Y';


                processor.process(
                    cardNumber,
                    expiry,
                    cvv,
                    amount,
                    international
                );


                break;
            }


            // =================================================
            // Transaction History
            // =================================================

            case 2:

                showTransactionHistory(store);

                break;


            // =================================================
            // Exit
            // =================================================

            case 3:

                std::cout
                    << "\nExiting payment system...\n";

                return 0;


            default:

                std::cout
                    << "\nInvalid choice. Try again.\n";
        }
    }
}