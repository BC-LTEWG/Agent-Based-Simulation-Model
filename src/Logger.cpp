#include <stdexcept>

#include "Logger.h"

LogPair::LogPair(std::string key, double value) :
    key{key}, value{value}
{}

LogPairS::LogPairS(std::string key, std::string value) :
    key{key}, value{value}
{}

std::ostream& operator<<(std::ostream& os, const LogPair& lp) {
    os << ",\"" << lp.key << "\":" << lp.value;
    return os;
}

std::ostream& operator<<(std::ostream& os, const LogPairS& lp) {
    os << ",\"" << lp.key << "\":\"" << lp.value << "\"";
    return os;
}

void force_compiler_to_generate_specializations() {
    Logger::log<LogPair>(
            Logger::ERROR,
            0,
            "code_gen",
            LogPair("x", 0)
            );
    Logger::log<LogPair, LogPair>(
            Logger::ERROR,
            0,
            "code_gen",
            LogPair("x", 0),
            LogPair("x", 0)
            );
    Logger::log<LogPair, LogPair, LogPair>(
            Logger::ERROR,
            0,
            "code_gen",
            LogPair("x", 0), LogPair("x", 0),
            LogPair("x", 0), LogPair("x", 0),
            LogPair("x", 0), LogPair("x", 0)
            );
    Logger::log<LogPair, LogPair, LogPair, LogPair>(
            Logger::ERROR,
            0,
            "code_gen",
            LogPair("x", 0), LogPair("x", 0),
            LogPair("x", 0), LogPair("x", 0),
            LogPair("x", 0), LogPair("x", 0),
            LogPair("x", 0), LogPair("x", 0)
            );
    Logger::log<LogPair, LogPair, LogPair, LogPair, LogPair>(
            Logger::ERROR,
            0,
            "code_gen",
            LogPair("x", 0), LogPair("x", 0),
            LogPair("x", 0), LogPair("x", 0),
            LogPair("x", 0), LogPair("x", 0),
            LogPair("x", 0), LogPair("x", 0),
            LogPair("x", 0), LogPair("x", 0)
            );
}
