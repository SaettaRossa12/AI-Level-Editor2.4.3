#include "ScriptParser.hpp"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <sstream>
#include <string_view>
#include <unordered_map>

namespace levelscribe {
namespace {

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

bool parseFloat(std::string const& text, float& value) {
    auto begin = text.data();
    auto end = begin + text.size();
    auto [ptr, err] = std::from_chars(begin, end, value);
    return err == std::errc() && ptr == end;
}

bool parseInt(std::string const& text, int& value) {
    auto begin = text.data();
    auto end = begin + text.size();
    auto [ptr, err] = std::from_chars(begin, end, value);
    return err == std::errc() && ptr == end;
}

std::vector<std::string> splitWords(std::string const& line) {
    std::istringstream stream(line);
    std::vector<std::string> words;
    std::string word;
    while (stream >> word) {
        words.push_back(word);
    }
    return words;
}

int orbId(std::string const& name) {
    static std::unordered_map<std::string, int> ids {
        {"yellow", 36},
        {"pink", 141},
        {"blue", 84},
        {"green", 1022},
        {"red", 1333},
        {"black", 1330}
    };
    auto found = ids.find(name);
    return found == ids.end() ? 36 : found->second;
}

int portalId(std::string const& name) {
    static std::unordered_map<std::string, int> ids {
        {"cube", 12},
        {"ship", 13},
        {"ball", 47},
        {"ufo", 111},
        {"wave", 660},
        {"robot", 745},
        {"spider", 1331}
    };
    auto found = ids.find(name);
    return found == ids.end() ? 12 : found->second;
}

int sectionLength(std::string const& name) {
    if (name == "short") return 6;
    if (name == "long") return 24;
    return 12;
}

}

ParseResult parseScript(std::string const& script) {
    ParseResult result;
    std::istringstream lines(script);
    std::string line;
    int lineNumber = 0;

    while (std::getline(lines, line)) {
        lineNumber++;
        auto comment = line.find('#');
        if (comment != std::string::npos) {
            line = line.substr(0, comment);
        }

        auto words = splitWords(line);
        if (words.empty()) {
            continue;
        }

        auto command = lower(words[0]);
        auto warn = [&](std::string const& message) {
            result.warnings.push_back("Line " + std::to_string(lineNumber) + ": " + message);
        };

        if (command == "theme" || command == "start" || command == "song") {
            continue;
        }

        if (command == "block") {
            if (words.size() < 3) {
                warn("block requires x and y.");
                continue;
            }
            float x = 0.0f;
            float y = 0.0f;
            int length = 1;
            if (!parseFloat(words[1], x) || !parseFloat(words[2], y)) {
                warn("block coordinates must be numbers.");
                continue;
            }
            if (words.size() >= 4 && !parseInt(words[3], length)) {
                warn("block length must be a number.");
                continue;
            }
            length = std::max(1, std::min(length, 64));
            for (int i = 0; i < length; i++) {
                result.commands.push_back({ObjectKind::Block, x + static_cast<float>(i), y, 1, 1, "block"});
            }
            continue;
        }

        if (command == "spike") {
            if (words.size() < 3) {
                warn("spike requires x and y.");
                continue;
            }
            float x = 0.0f;
            float y = 0.0f;
            if (!parseFloat(words[1], x) || !parseFloat(words[2], y)) {
                warn("spike coordinates must be numbers.");
                continue;
            }
            result.commands.push_back({ObjectKind::Spike, x, y, 8, 1, "spike"});
            continue;
        }

        if (command == "orb") {
            if (words.size() < 4) {
                warn("orb requires color, x and y.");
                continue;
            }
            float x = 0.0f;
            float y = 0.0f;
            if (!parseFloat(words[2], x) || !parseFloat(words[3], y)) {
                warn("orb coordinates must be numbers.");
                continue;
            }
            auto color = lower(words[1]);
            result.commands.push_back({ObjectKind::Orb, x, y, orbId(color), 1, color});
            continue;
        }

        if (command == "portal") {
            if (words.size() < 4) {
                warn("portal requires mode, x and y.");
                continue;
            }
            float x = 0.0f;
            float y = 0.0f;
            if (!parseFloat(words[2], x) || !parseFloat(words[3], y)) {
                warn("portal coordinates must be numbers.");
                continue;
            }
            auto mode = lower(words[1]);
            result.commands.push_back({ObjectKind::Portal, x, y, portalId(mode), 1, mode});
            continue;
        }

        if (command == "section") {
            if (words.size() < 4) {
                warn("section requires x, y and mode.");
                continue;
            }
            float x = 0.0f;
            float y = 0.0f;
            if (!parseFloat(words[1], x) || !parseFloat(words[2], y)) {
                warn("section coordinates must be numbers.");
                continue;
            }
            auto mode = lower(words[3]);
            auto size = words.size() >= 5 ? sectionLength(lower(words[4])) : 12;
            for (int i = 0; i < size; i++) {
                result.commands.push_back({ObjectKind::Block, x + static_cast<float>(i), y, 1, 1, mode});
            }
            result.commands.push_back({ObjectKind::Portal, x, y + 2.0f, portalId(mode), 1, mode});
            continue;
        }

        warn("unknown command `" + words[0] + "`.");
    }

    return result;
}

}
