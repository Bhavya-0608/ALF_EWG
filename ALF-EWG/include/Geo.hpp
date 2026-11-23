// Unit I: call by reference / return by reference
#pragma once
#include <cmath>
#include <cstddef>

namespace DisasterCore {
namespace Sensors {

struct GeoCoordinate { double lat, lon, elevM; };

// Call by reference + return by reference: mutates in place and allows chaining.
inline GeoCoordinate& translate(GeoCoordinate& c, double dLat, double dLon) {
    c.lat += dLat;
    c.lon += dLon;
    return c;
}

// Call by reference for OUTPUT parameters (x, y in metres from origin).
inline void toLocalGrid(const GeoCoordinate& origin, const GeoCoordinate& p,
                        double& xMetres, double& yMetres) {
    const double kPi = 3.14159265358979323846;
    xMetres = (p.lon - origin.lon) * 111320.0 * std::cos(origin.lat * kPi / 180.0);
    yMetres = (p.lat - origin.lat) * 110574.0;
}

// Running telemetry state accumulated without copying.
struct TelemetryState { double sum = 0, peak = 0; std::size_t n = 0; };

inline TelemetryState& accumulate(TelemetryState& s, double v) {
    s.sum += v;
    if (v > s.peak) s.peak = v;
    ++s.n;
    return s;
}

} // namespace Sensors
} // namespace DisasterCore
