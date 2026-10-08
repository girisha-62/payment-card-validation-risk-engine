#include "card/card_validator.hpp"

#include <cctype>
#include <chrono>
#include <ctime>
#include <algorithm>

namespace {
bool startsWith(const std::string& value, const std::string& prefix) {
    return value.rfind(prefix, 0) == 0;
}
}

std::string CardValidator::digitsOnly(const std::string& input) {
    std::string result;
    for (char c : input) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            result += c;
        }
    }
    return result;
}

CardNetwork CardValidator::identifyNetwork(const std::string& input) {
    const auto card = digitsOnly(input);

    if (card.size() == 16 && card[0] == '4') {
        return CardNetwork::VISA;
    }

    if (card.size() == 16) {
        int firstTwo = std::stoi(card.substr(0, 2));
        if (firstTwo >= 51 && firstTwo <= 55) {
            return CardNetwork::MASTERCARD;
        }
        int firstFour = std::stoi(card.substr(0, 4));
        if (firstFour >= 2221 && firstFour <= 2720) {
            return CardNetwork::MASTERCARD;
        }
    }

    if ((card.size() == 15) &&
        (startsWith(card, "34") || startsWith(card, "37"))) {
        return CardNetwork::AMERICAN_EXPRESS;
    }

    if (card.size() == 16 &&
        (startsWith(card, "60") || startsWith(card, "65") ||
         startsWith(card, "81") || startsWith(card, "82") ||
         startsWith(card, "508"))) {
        return CardNetwork::RUPAY;
    }

    return CardNetwork::UNKNOWN;
}

bool CardValidator::luhnCheck(const std::string& input) {
    const auto card = digitsOnly(input);
    if (card.empty()) return false;

    int sum = 0;
    bool doubleDigit = false;

    for (auto it = card.rbegin(); it != card.rend(); ++it) {
        int digit = *it - '0';

        if (doubleDigit) {
            digit *= 2;
            if (digit > 9) digit -= 9;
        }

        sum += digit;
        doubleDigit = !doubleDigit;
    }

    return sum % 10 == 0;
}

bool CardValidator::validExpiry(const std::string& expiryMMYY) {
    if (expiryMMYY.size() != 4) return false;

    for (char c : expiryMMYY) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    }

    int month = (expiryMMYY[0] - '0') * 10 + expiryMMYY[1] - '0';
    int year = 2000 + (expiryMMYY[2] - '0') * 10 + expiryMMYY[3] - '0';

    if (month < 1 || month > 12) return false;

    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tmNow{};

#ifdef _WIN32
    localtime_s(&tmNow, &t);
#else
    localtime_r(&t, &tmNow);
#endif

    int currentYear = tmNow.tm_year + 1900;
    int currentMonth = tmNow.tm_mon + 1;

    return year > currentYear ||
           (year == currentYear && month >= currentMonth);
}

bool CardValidator::validCvv(
    const std::string& cvv,
    CardNetwork network) {

    const size_t expected =
        network == CardNetwork::AMERICAN_EXPRESS ? 4 : 3;

    if (cvv.size() != expected) return false;

    for (char c : cvv) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    }

    return true;
}

CardValidationResult CardValidator::validate(
    const std::string& cardNumber,
    const std::string& expiryMMYY,
    const std::string& cvv) const {

    CardValidationResult result;

    result.network = identifyNetwork(cardNumber);
    result.luhnValid = luhnCheck(cardNumber);
    result.expiryValid = validExpiry(expiryMMYY);
    result.cvvValid = validCvv(cvv, result.network);

    result.valid =
        result.network != CardNetwork::UNKNOWN &&
        result.luhnValid &&
        result.expiryValid &&
        result.cvvValid;

    result.message =
        result.valid
        ? "Card validation successful"
        : "Card validation failed";

    return result;
}

std::string CardValidator::networkName(CardNetwork network) {
    switch (network) {
        case CardNetwork::VISA: return "VISA";
        case CardNetwork::MASTERCARD: return "MASTERCARD";
        case CardNetwork::AMERICAN_EXPRESS: return "AMERICAN_EXPRESS";
        case CardNetwork::RUPAY: return "RUPAY";
        default: return "UNKNOWN";
    }
}
