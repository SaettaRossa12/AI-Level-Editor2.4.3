#pragma once

#include <string>

namespace levelscribe {

struct IntegrityReport {
    bool suspicious = false;
    std::string reason;
    std::string settingsHash;
};

IntegrityReport collectIntegrityReport();

}
