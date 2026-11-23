// Unit IV: custom stream manipulators and functors for STL algorithms
#pragma once
#include <cmath>
#include <iomanip>
#include <map>
#include <ostream>
#include <string>
#include "HazardIndex.hpp"
#include "IncidentLogger.hpp"

namespace DisasterCore {
namespace Alerts {

// ---- custom manipulators ----
struct ColWidth { int w; };                                     // parameterised manipulator
inline ColWidth col(int w) { return ColWidth{w}; }
inline std::ostream& operator<<(std::ostream& os, ColWidth c) { return os << std::left << std::setw(c.w); }

inline std::ostream& tableRule(std::ostream& os)                // simple manipulator
{ return os << std::string(66, '-') << '\n'; }

inline std::ostream& alertHeader(std::ostream& os) {
    os << tableRule << col(6) << "Rank" << col(10) << "Station" << col(14) << "Region"
       << col(10) << "Hazard" << col(12) << "Level" << "EvacScore\n" << tableRule;
    return os;
}

// ---- functor: priority-based evacuation routing (hazard weighted by population at risk) ----
class EvacuationPriority {
    const std::map<std::string, int>* population_;
public:
    explicit EvacuationPriority(const std::map<std::string, int>& pop) : population_(&pop) {}
    double score(const IncidentRecord& r) const {
        std::map<std::string, int>::const_iterator it = population_->find(r.region);
        double pop = (it == population_->end()) ? 0.0 : it->second;
        return r.hazard * (1.0 + std::log10(1.0 + pop));
    }
    bool operator()(const IncidentRecord& a, const IncidentRecord& b) const {
        return score(a) > score(b);                             // highest priority first
    }
};

// ---- functor used with std::for_each: prints one dispatch row ----
class DispatchPrinter {
    std::ostream& os_;
    const EvacuationPriority& pr_;
    int rank_;
public:
    DispatchPrinter(std::ostream& os, const EvacuationPriority& pr) : os_(os), pr_(pr), rank_(0) {}
    void operator()(const IncidentRecord& r) {
        std::ios_base::fmtflags f = os_.flags();
        std::streamsize p = os_.precision();
        os_ << col(6) << ++rank_ << col(10) << r.stationId << col(14) << r.region
            << col(10) << std::fixed << std::setprecision(1) << r.hazard
            << col(12) << toString(static_cast<AlertLevel>(r.level))
            << pr_.score(r) << '\n';
        os_.flags(f); os_.precision(p);
    }
};

} // namespace Alerts
} // namespace DisasterCore
