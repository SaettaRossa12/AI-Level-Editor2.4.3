#pragma once

#include <string>
#include <vector>

namespace levelscribe {

enum class ObjectKind {
    Block,
    Spike,
    Orb,
    Portal,
    SectionMarker
};

struct LevelCommand {
    ObjectKind kind;
    float x = 0.0f;
    float y = 0.0f;
    int objectId = 0;
    int length = 1;
    std::string tag;
};

struct ParseResult {
    std::vector<LevelCommand> commands;
    std::vector<std::string> warnings;
};

ParseResult parseScript(std::string const& script);

}
