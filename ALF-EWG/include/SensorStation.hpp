// Unit II: ADT with const correctness, static members, operator overloading
// Unit I : friend class TelemetryAuditor
#pragma once
#include <iosfwd>
#include <string>
#include <vector>
#include "Geo.hpp"
#include "TelemetryBuffer.hpp"

namespace DisasterCore {
namespace Sensors {

class SensorStation;
class TelemetryAuditor;

struct ZoneMember { int id; std::string name; double hazard; };

// Regional zone vector produced by operator+ on stations.
class RegionalZone {
    std::string name_;
    std::vector<ZoneMember> members_;
public:
    explicit RegionalZone(const std::string& name = "Zone") : name_(name) {}
    RegionalZone operator+(const SensorStation& s) const;   // zone + station
    RegionalZone operator+(const RegionalZone& z) const;    // zone + zone (merge)
    double peakHazard() const;
    double meanHazard() const;
    std::size_t size() const { return members_.size(); }
    friend std::ostream& operator<<(std::ostream& os, const RegionalZone& z);
};

class SensorStation {
    int           id_;
    std::string   name_;
    GeoCoordinate loc_;
    // raw private metrics (only accessible via friend TelemetryAuditor)
    double rain_, slope_, saturation_, vibration_;
    double hazard_;
    TelemetryBuffer buffer_;     // composition: owns dynamic memory (deep-copied)
    TelemetryState  state_;

    static int activeStations_;  // network-wide count

public:
    SensorStation();
    SensorStation(int id, const std::string& name, const GeoCoordinate& loc,
                  std::size_t bufferCapacity = 8);
    SensorStation(const SensorStation& other);
    SensorStation& operator=(const SensorStation& other) = default;
    ~SensorStation();

    void update(double rainMmHr, double slopeDeg, double saturationPct, double vibrationHz);

    int                  id()       const { return id_; }
    const std::string&   name()     const { return name_; }
    double               hazard()   const { return hazard_; }
    const GeoCoordinate& location() const { return loc_; }
    const TelemetryBuffer& history() const { return buffer_; }

    static int activeStations() { return activeStations_; }

    bool operator>(const SensorStation& rhs) const { return hazard_ > rhs.hazard_; }
    RegionalZone operator+(const SensorStation& rhs) const;

    friend std::ostream& operator<<(std::ostream& os, const SensorStation& s);
    friend std::istream& operator>>(std::istream& is, SensorStation& s);
    friend class TelemetryAuditor;
};

// Friend class: reads raw private metrics without public getters.
class TelemetryAuditor {
public:
    void audit(const SensorStation& s, std::ostream& os) const;
};

} // namespace Sensors
} // namespace DisasterCore
