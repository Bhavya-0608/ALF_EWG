// Unit I: namespace DisasterCore::Alerts, inline functions, function overloading
#pragma once
#include <algorithm>

namespace DisasterCore {
namespace Alerts {

enum class AlertLevel : int { Normal = 0, Watch = 1, Warning = 2, Emergency = 3 };

inline const char* toString(AlertLevel l) {
    switch (l) {
        case AlertLevel::Normal:    return "NORMAL";
        case AlertLevel::Watch:     return "WATCH";
        case AlertLevel::Warning:   return "WARNING";
        case AlertLevel::Emergency: return "EMERGENCY";
    }
    return "UNKNOWN";
}

// Strong types let the overload set differ by *type* (not only by arity).
struct Saturation { double pct; };   // soil saturation 0..100 %
struct Vibration  { double hz;  };   // seismic vibration frequency in Hz

// Inline helper used in the real-time path (no call overhead).
inline double normalise(double v, double lo, double hi) {
    return std::max(0.0, std::min(1.0, (v - lo) / (hi - lo)));
}

// Overload 1: rainfall rate (mm/hr) + slope angle (deg)
inline double computeHazardIndex(double rainfallMmHr, double slopeDeg) {
    return 100.0 * (0.6 * normalise(rainfallMmHr, 0, 100) +
                    0.4 * normalise(slopeDeg, 0, 60));
}
// Overload 2: soil saturation + vibration frequency
inline double computeHazardIndex(Saturation s, Vibration v) {
    return 100.0 * (0.7 * normalise(s.pct, 0, 100) +
                    0.3 * normalise(v.hz, 0, 25));
}
// Overload 3: river level (m) + rainfall rate (mm/hr) + soil saturation -> flash-flood index
inline double computeHazardIndex(double riverLevelM, double rainfallMmHr, double soilSatPct) {
    return 100.0 * (0.5 * normalise(riverLevelM, 0, 8) +
                    0.3 * normalise(rainfallMmHr, 0, 100) +
                    0.2 * normalise(soilSatPct, 0, 100));
}

inline AlertLevel classify(double hazard) {
    if (hazard >= 75) return AlertLevel::Emergency;
    if (hazard >= 55) return AlertLevel::Warning;
    if (hazard >= 35) return AlertLevel::Watch;
    return AlertLevel::Normal;
}

} // namespace Alerts
} // namespace DisasterCore
