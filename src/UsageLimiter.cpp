#include "UsageLimiter.hpp"

#include <Geode/Geode.hpp>

#include <algorithm>
#include <ctime>

using namespace geode::prelude;

namespace levelscribe {
namespace {

std::tm currentLocalTime() {
    std::time_t now = std::time(nullptr);
    std::tm out {};
#ifdef _WIN32
    localtime_s(&out, &now);
#else
    localtime_r(&now, &out);
#endif
    return out;
}

long long nowUnix() {
    return static_cast<long long>(std::time(nullptr));
}

}

UsageLimiter::UsageLimiter(std::string storageKey) : m_storageKey(std::move(storageKey)) {}

UsageState UsageLimiter::load() const {
    auto now = currentLocalTime();
    UsageState state;
    state.year = Mod::get()->getSavedValue<int>(m_storageKey + ".year", now.tm_year + 1900);
    state.month = Mod::get()->getSavedValue<int>(m_storageKey + ".month", now.tm_mon + 1);
    state.naturalPromptsUsed = Mod::get()->getSavedValue<int>(m_storageKey + ".naturalPromptsUsed", 0);
    state.bannedUntilUnix = Mod::get()->getSavedValue<long long>(m_storageKey + ".bannedUntilUnix", 0);

    if (state.year != now.tm_year + 1900 || state.month != now.tm_mon + 1) {
        state.year = now.tm_year + 1900;
        state.month = now.tm_mon + 1;
        state.naturalPromptsUsed = 0;
        save(state);
    }

    return state;
}

void UsageLimiter::save(UsageState const& state) const {
    Mod::get()->setSavedValue(m_storageKey + ".year", state.year);
    Mod::get()->setSavedValue(m_storageKey + ".month", state.month);
    Mod::get()->setSavedValue(m_storageKey + ".naturalPromptsUsed", state.naturalPromptsUsed);
    Mod::get()->setSavedValue(m_storageKey + ".bannedUntilUnix", state.bannedUntilUnix);
}

int UsageLimiter::remainingNaturalPrompts() const {
    auto state = load();
    if (state.bannedUntilUnix > nowUnix()) {
        return 0;
    }
    return std::max(0, kMonthlyNaturalPromptLimit - state.naturalPromptsUsed);
}

bool UsageLimiter::consumeNaturalPrompt() {
    auto state = load();
    if (state.bannedUntilUnix > nowUnix()) {
        return false;
    }
    if (state.naturalPromptsUsed >= kMonthlyNaturalPromptLimit) {
        return false;
    }
    state.naturalPromptsUsed++;
    save(state);
    return true;
}

bool UsageLimiter::isBanned() const {
    return load().bannedUntilUnix > nowUnix();
}

void UsageLimiter::markTamperDetected() const {
    auto state = load();
    state.bannedUntilUnix = nowUnix() + static_cast<long long>(kBanDays) * 24LL * 60LL * 60LL;
    save(state);
}

}
