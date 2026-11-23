// Unit III: abstract base, virtual inheritance (diamond), containership, polymorphism
#pragma once
#include <cstdint>
#include <string>
#include "Exceptions.hpp"
#include "Geo.hpp"
#include "HazardIndex.hpp"

namespace DisasterCore {
namespace Sensors {

// Simulation helpers (seeded for reproducible runs)
void   seedSimulation(unsigned seed);

// ---------------- Root of the diamond ----------------
class DisasterTelemetryDevice {
protected:
    int         deviceId_;
    std::string firmware_;
public:
    explicit DisasterTelemetryDevice(int id = 0, const std::string& fw = "fw-1.0")
        : deviceId_(id), firmware_(fw) {}
    virtual ~DisasterTelemetryDevice() {}
    int deviceId() const { return deviceId_; }
    const std::string& firmware() const { return firmware_; }
    virtual std::string describe() const { return "Device#" + std::to_string(deviceId_); }
};

// ---------------- Containership: independent object composed inside nodes ----------------
class PowerSubsystem {
    double batteryPct_;
    double solarWatts_;
public:
    explicit PowerSubsystem(double battery = 100.0, double solarW = 0.0)
        : batteryPct_(battery), solarWatts_(solarW) {}
    void drain(double pct)  { batteryPct_ = batteryPct_ - pct < 0 ? 0 : batteryPct_ - pct; }
    void charge(double pct) { batteryPct_ = batteryPct_ + pct > 100 ? 100 : batteryPct_ + pct; }
    double batteryPct() const { return batteryPct_; }
    bool   depleted()   const { return batteryPct_ <= 0.0; }
};

// ---------------- Abstract base ----------------
class BaseSensorNode : public virtual DisasterTelemetryDevice {
protected:
    PowerSubsystem power_;          // HAS-A (aggregation), not IS-A
    std::string    label_;
    GeoCoordinate  loc_;
    double         last_;
    TelemetryState state_;
    bool           hasOverride_;
    double         override_;

    double sample(double mean, double sd);                  // simulated measurement
    double commit(double value, double physicalMax);        // validates + accumulates
public:
    BaseSensorNode(int id, const std::string& label, const GeoCoordinate& loc,
                   double battery = 100.0);
    virtual ~BaseSensorNode();

    virtual double      readTelemetry() = 0;                // pure virtual
    virtual bool        evaluateThreshold() const = 0;      // pure virtual
    virtual const char* kind() const = 0;

    void injectFault(double value) { hasOverride_ = true; override_ = value; } // testing hook

    const std::string&    label() const { return label_; }
    double                lastReading() const { return last_; }
    const PowerSubsystem& power() const { return power_; }
    const GeoCoordinate&  location() const { return loc_; }
};

// ---------------- Simple concrete nodes ----------------
class SoilMoistureNode : public BaseSensorNode {
    double baseline_;
public:
    SoilMoistureNode(int id, const std::string& l, const GeoCoordinate& g, double baseline,
                     double battery = 100.0)
        : DisasterTelemetryDevice(id, "soil-fw"), BaseSensorNode(id, l, g, battery),
          baseline_(baseline) {}
    double readTelemetry() override { return commit(sample(baseline_, 2.0), 100.0); }
    bool evaluateThreshold() const override { return last_ > 80.0; }
    const char* kind() const override { return "SoilMoisture(%)"; }
};

class SeismicGeophone : public BaseSensorNode {
    double baseline_;
public:
    SeismicGeophone(int id, const std::string& l, const GeoCoordinate& g, double baseline,
                    double battery = 100.0)
        : DisasterTelemetryDevice(id, "geo-fw"), BaseSensorNode(id, l, g, battery),
          baseline_(baseline) {}
    double readTelemetry() override { return commit(sample(baseline_, 0.8), 200.0); }
    bool evaluateThreshold() const override { return last_ > 15.0; }
    const char* kind() const override { return "Seismic(Hz)"; }
};

class AcousticTiltmeter : public BaseSensorNode {
    double baseline_;
public:
    AcousticTiltmeter(int id, const std::string& l, const GeoCoordinate& g, double baseline,
                      double battery = 100.0)
        : DisasterTelemetryDevice(id, "tilt-fw"), BaseSensorNode(id, l, g, battery),
          baseline_(baseline) {}
    double readTelemetry() override { return commit(sample(baseline_, 0.3), 90.0); }
    bool evaluateThreshold() const override { return last_ > 5.0; }
    const char* kind() const override { return "Tilt(deg)"; }
};

class RiverSonarGauge : public BaseSensorNode {
    double baseline_;
public:
    RiverSonarGauge(int id, const std::string& l, const GeoCoordinate& g, double baseline,
                    double battery = 100.0)
        : DisasterTelemetryDevice(id, "sonar-fw"), BaseSensorNode(id, l, g, battery),
          baseline_(baseline) {}
    double readTelemetry() override { return commit(sample(baseline_, 0.15), 20.0); }
    bool evaluateThreshold() const override { return last_ > 4.5; }
    const char* kind() const override { return "RiverLevel(m)"; }
};

// ---------------- Telemetry packet with checksum ----------------
struct TelemetryPacket { std::int32_t nodeId; double value; std::uint32_t checksum; };
std::uint32_t packetChecksum(std::int32_t nodeId, double value);
void verifyPacket(const TelemetryPacket& p);   // throws TelemetryChecksumMismatch

// ---------------- Intermediate derivations (virtual inheritance) ----------------
class SolarPoweredNode : public virtual BaseSensorNode {
protected:
    double panelEfficiency_;
public:
    SolarPoweredNode(int id, const std::string& l, const GeoCoordinate& g, double eff)
        : DisasterTelemetryDevice(id), BaseSensorNode(id, l, g), panelEfficiency_(eff) {}
    void harvest(double sunHours) { power_.charge(panelEfficiency_ * sunHours * 10.0); }
};

class WirelessLoRaTransceiver : public virtual BaseSensorNode {
protected:
    int      rssiDbm_;
    unsigned packetsSent_;
public:
    WirelessLoRaTransceiver(int id, const std::string& l, const GeoCoordinate& g, int rssi)
        : DisasterTelemetryDevice(id), BaseSensorNode(id, l, g), rssiDbm_(rssi), packetsSent_(0) {}
    TelemetryPacket transmit(double value);
    unsigned packetsSent() const { return packetsSent_; }
};

// ---------------- Derived leaf ----------------
class AutonomousHybridProbe : public SolarPoweredNode, public WirelessLoRaTransceiver {
    double satBaseline_, vibBaseline_;
public:
    AutonomousHybridProbe(int id, const std::string& l, const GeoCoordinate& g,
                          double satBase, double vibBase, double eff = 0.2, int rssi = -95);
    double readTelemetry() override;
    bool evaluateThreshold() const override { return last_ >= 60.0; }
    const char* kind() const override { return "HybridProbe(idx)"; }
    std::string describe() const override;
    ~AutonomousHybridProbe() override {}
};

} // namespace Sensors
} // namespace DisasterCore
