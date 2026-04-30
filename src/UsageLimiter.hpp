#pragma once

#include <string>

namespace levelscribe {

struct UsageState {
    int year = 0;
    int month = 0;
    int naturalPromptsUsed = 0;
    long long bannedUntilUnix = 0;
};

class UsageLimiter {
public:
    static constexpr int kMonthlyNaturalPromptLimit = 3;
    static constexpr int kBanDays = 14;

    explicit UsageLimiter(std::string storageKey);

    UsageState load() const;
    void save(UsageState const& state) const;
    int remainingNaturalPrompts() const;
    bool consumeNaturalPrompt();
    bool isBanned() const;
    void markTamperDetected() const;

private:
    std::string m_storageKey;
};

}
