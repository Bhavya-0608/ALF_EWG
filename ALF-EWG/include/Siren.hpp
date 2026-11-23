// Unit II: static data members / static member functions for global alert state
#pragma once
#include <string>
#include "HazardIndex.hpp"

namespace DisasterCore {
namespace Alerts {

class EarlyWarningSiren {
    int         id_;
    std::string region_;
    AlertLevel  current_;

    static AlertLevel globalLevel_;       // highest level raised by any siren
    static int        totalActivations_;  // network-wide activation counter
public:
    EarlyWarningSiren(int id, const std::string& region)
        : id_(id), region_(region), current_(AlertLevel::Normal) {}

    void trigger(AlertLevel lvl);
    void silence() { current_ = AlertLevel::Normal; }

    int                id()     const { return id_; }
    const std::string& region() const { return region_; }
    AlertLevel         level()  const { return current_; }

    static AlertLevel globalLevel()      { return globalLevel_; }
    static int        totalActivations() { return totalActivations_; }
    static void       resetGlobal()      { globalLevel_ = AlertLevel::Normal; totalActivations_ = 0; }
};

} // namespace Alerts
} // namespace DisasterCore
