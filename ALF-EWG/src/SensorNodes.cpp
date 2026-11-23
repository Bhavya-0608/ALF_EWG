#include "SensorNodes.hpp"
#include <cstring>
#include <random>

namespace DisasterCore {
namespace Sensors {

namespace {
std::mt19937& rng() { static std::mt19937 g(42); return g; }
}
void seedSimulation(unsigned seed) { rng().seed(seed); }

BaseSensorNode::BaseSensorNode(int id, const std::string& label, const GeoCoordinate& loc,
                               double battery)
    : DisasterTelemetryDevice(id), power_(battery), label_(label), loc_(loc),
      last_(0), state_(), hasOverride_(false), override_(0) {}

BaseSensorNode::~BaseSensorNode() {}

double BaseSensorNode::sample(double mean, double sd) {
    if (hasOverride_) { hasOverride_ = false; return override_; }
    std::normal_distribution<double> d(mean, sd);
    return d(rng());
}

double BaseSensorNode::commit(double value, double physicalMax) {
    power_.drain(1.0);                                   // every sweep costs 1% battery
    if (power_.depleted())
        throw SensorFailureException(deviceId(), "battery depleted at '" + label_ + "'");
    if (value > physicalMax)
        throw ThresholdOverflowException(value, physicalMax);
    if (value < 0) value = 0;
    last_ = value;
    accumulate(state_, value);
    return last_;
}

// ---- packets ----
std::uint32_t packetChecksum(std::int32_t id, double v) {      // FNV-1a over id + value bytes
    std::uint32_t h = 2166136261u;
    unsigned char buf[sizeof(id) + sizeof(v)];
    std::memcpy(buf, &id, sizeof(id));
    std::memcpy(buf + sizeof(id), &v, sizeof(v));
    for (unsigned char c : buf) { h ^= c; h *= 16777619u; }
    return h;
}
void verifyPacket(const TelemetryPacket& p) {
    std::uint32_t actual = packetChecksum(p.nodeId, p.value);
    if (actual != p.checksum) throw TelemetryChecksumMismatch(p.checksum, actual);
}

TelemetryPacket WirelessLoRaTransceiver::transmit(double value) {
    if (rssiDbm_ < -120)
        throw SensorFailureException(deviceId(), "LoRa link lost (RSSI too low)");
    ++packetsSent_;
    TelemetryPacket p{deviceId(), value, packetChecksum(deviceId(), value)};
    return p;
}

// ---- hybrid probe: most-derived class initialises ALL virtual bases ----
AutonomousHybridProbe::AutonomousHybridProbe(int id, const std::string& l,
        const GeoCoordinate& g, double satBase, double vibBase, double eff, int rssi)
    : DisasterTelemetryDevice(id, "hybrid-fw-2.1"),
      BaseSensorNode(id, l, g, 80.0),
      SolarPoweredNode(id, l, g, eff),
      WirelessLoRaTransceiver(id, l, g, rssi),
      satBaseline_(satBase), vibBaseline_(vibBase) {}

double AutonomousHybridProbe::readTelemetry() {
    using namespace Alerts;
    double idx = computeHazardIndex(Saturation{sample(satBaseline_, 3.0)},
                                    Vibration{sample(vibBaseline_, 0.5)});
    commit(idx, 100.0);
    harvest(2.0);                 // solar top-up (from SolarPoweredNode)
    transmit(last_);              // uplink (from WirelessLoRaTransceiver)
    return last_;
}

std::string AutonomousHybridProbe::describe() const {
    return "HybridProbe#" + std::to_string(deviceId()) + " [" + firmware_ + ", sent " +
           std::to_string(packetsSent_) + " pkts]";
}

} // namespace Sensors
} // namespace DisasterCore
