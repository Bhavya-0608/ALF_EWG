#include "SensorStation.hpp"
#include "HazardIndex.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>

namespace DisasterCore {
namespace Sensors {

int SensorStation::activeStations_ = 0;

SensorStation::SensorStation() : SensorStation(0, "unnamed", GeoCoordinate{0, 0, 0}) {}

SensorStation::SensorStation(int id, const std::string& name, const GeoCoordinate& loc,
                             std::size_t cap)
    : id_(id), name_(name), loc_(loc), rain_(0), slope_(0), saturation_(0),
      vibration_(0), hazard_(0), buffer_(cap), state_() {
    ++activeStations_;
}

SensorStation::SensorStation(const SensorStation& o)
    : id_(o.id_), name_(o.name_), loc_(o.loc_), rain_(o.rain_), slope_(o.slope_),
      saturation_(o.saturation_), vibration_(o.vibration_), hazard_(o.hazard_),
      buffer_(o.buffer_), state_(o.state_) {          // buffer_ deep-copied
    ++activeStations_;
}

SensorStation::~SensorStation() { --activeStations_; }

void SensorStation::update(double rain, double slope, double sat, double vib) {
    using namespace Alerts;
    rain_ = rain; slope_ = slope; saturation_ = sat; vibration_ = vib;
    // two different overloads of computeHazardIndex are blended
    hazard_ = 0.5 * computeHazardIndex(rain, slope) +
              0.5 * computeHazardIndex(Saturation{sat}, Vibration{vib});
    buffer_.push(hazard_);
    accumulate(state_, hazard_);
}

RegionalZone SensorStation::operator+(const SensorStation& rhs) const {
    RegionalZone z("Zone(" + name_ + "+" + rhs.name_ + ")");
    return z + *this + rhs;
}

// ---- RegionalZone ----
RegionalZone RegionalZone::operator+(const SensorStation& s) const {
    RegionalZone r(*this);
    r.members_.push_back(ZoneMember{s.id(), s.name(), s.hazard()});
    return r;
}
RegionalZone RegionalZone::operator+(const RegionalZone& z) const {
    RegionalZone r(*this);
    r.members_.insert(r.members_.end(), z.members_.begin(), z.members_.end());
    return r;
}
double RegionalZone::peakHazard() const {
    double p = 0;
    for (const ZoneMember& m : members_) p = std::max(p, m.hazard);
    return p;
}
double RegionalZone::meanHazard() const {
    if (members_.empty()) return 0;
    double s = 0;
    for (const ZoneMember& m : members_) s += m.hazard;
    return s / static_cast<double>(members_.size());
}
std::ostream& operator<<(std::ostream& os, const RegionalZone& z) {
    std::ios_base::fmtflags f = os.flags();      // save/restore: don't leak formatting state
    std::streamsize p = os.precision();
    os << z.name_ << " [" << z.members_.size() << " stations, peak="
       << std::fixed << std::setprecision(1) << z.peakHazard()
       << ", mean=" << z.meanHazard() << "]";
    os.flags(f); os.precision(p);
    return os;
}

// ---- stream operators: "id name lat lon elev hazard" (same format for console & file) ----
std::ostream& operator<<(std::ostream& os, const SensorStation& s) {
    os << s.id_ << ' ' << s.name_ << ' ' << s.loc_.lat << ' ' << s.loc_.lon << ' '
       << s.loc_.elevM << ' ' << s.hazard_;
    return os;
}
std::istream& operator>>(std::istream& is, SensorStation& s) {
    int id; std::string name; GeoCoordinate g; double h;
    if (is >> id >> name >> g.lat >> g.lon >> g.elevM >> h) {
        s.id_ = id; s.name_ = name; s.loc_ = g; s.hazard_ = h;
    }
    return is;
}

void TelemetryAuditor::audit(const SensorStation& s, std::ostream& os) const {
    os << "[AUDIT] station " << s.id_ << " (" << s.name_ << ") raw metrics: rain="
       << s.rain_ << " mm/hr, slope=" << s.slope_ << " deg, saturation="
       << s.saturation_ << " %, vibration=" << s.vibration_ << " Hz, samples="
       << s.state_.n << ", peak=" << s.state_.peak << "\n";
}

} // namespace Sensors
} // namespace DisasterCore
