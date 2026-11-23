#include "Siren.hpp"

namespace DisasterCore {
namespace Alerts {

AlertLevel EarlyWarningSiren::globalLevel_ = AlertLevel::Normal;
int        EarlyWarningSiren::totalActivations_ = 0;

void EarlyWarningSiren::trigger(AlertLevel lvl) {
    current_ = lvl;
    if (lvl != AlertLevel::Normal) ++totalActivations_;
    if (static_cast<int>(lvl) > static_cast<int>(globalLevel_)) globalLevel_ = lvl;
}

} // namespace Alerts
} // namespace DisasterCore
