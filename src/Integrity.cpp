#include "Integrity.hpp"

#include <Geode/Geode.hpp>

#include <functional>
#include <sstream>

using namespace geode::prelude;

namespace levelscribe {
namespace {

std::string hexHash(std::string const& input) {
    std::stringstream stream;
    stream << std::hex << std::hash<std::string>{}(input);
    return stream.str();
}

}

IntegrityReport collectIntegrityReport() {
    IntegrityReport report;
    auto serverUrl = Mod::get()->getSettingValue<std::string>("server-url");
    auto tutorial = Mod::get()->getSettingValue<bool>("show-tutorial-on-start");
    auto material = serverUrl + "|" + (tutorial ? "1" : "0") + "|winza.levelscribe-ai/v0.1.0";
    report.settingsHash = hexHash(material);

    if (serverUrl.empty()) {
        report.suspicious = true;
        report.reason = "Server URL is empty.";
    }

    return report;
}

}
