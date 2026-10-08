#pragma once

#include <string>

enum class CardNetwork {
    VISA,
    MASTERCARD,
    AMERICAN_EXPRESS,
    RUPAY,
    UNKNOWN
};

struct CardValidationResult {
    bool valid{false};
    CardNetwork network{CardNetwork::UNKNOWN};
    bool luhnValid{false};
    bool expiryValid{false};
    bool cvvValid{false};
    std::string message;
};

class CardValidator {
public:
    CardValidationResult validate(
        const std::string& cardNumber,
        const std::string& expiryMMYY,
        const std::string& cvv) const;

    static CardNetwork identifyNetwork(const std::string& cardNumber);
    static bool luhnCheck(const std::string& cardNumber);
    static bool validExpiry(const std::string& expiryMMYY);
    static bool validCvv(const std::string& cvv, CardNetwork network);
    static std::string networkName(CardNetwork network);

private:
    static std::string digitsOnly(const std::string& input);
};
