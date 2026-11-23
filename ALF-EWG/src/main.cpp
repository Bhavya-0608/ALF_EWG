// ALF-EWG : Autonomous Landslide & Flash-Flood Early Warning Grid
// CIC-211 Object-Oriented Programming using C++ - Project (Option 1)
//
// main.cpp is a demonstration driver: one function per syllabus unit.
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "Exceptions.hpp"
#include "HazardIndex.hpp"
#include "IncidentLogger.hpp"
#include "Reporting.hpp"
#include "SensorNodes.hpp"
#include "SensorStation.hpp"
#include "Siren.hpp"

using namespace DisasterCore;
namespace S = DisasterCore::Sensors;
namespace A = DisasterCore::Alerts;

static void banner(const std::string& t) {
    std::cout << "\n==================== " << t << " ====================\n";
}

// =====================================================================
// UNIT I : namespaces, inline, overloading, call/return by reference, friend
// =====================================================================
static void unitOne() {
    banner("UNIT I: Core foundations & call semantics");

    // function overloading + inline functions (three computeHazardIndex overloads)
    std::cout << "Hazard (rain 85mm/hr, slope 40deg)     = "
              << A::computeHazardIndex(85.0, 40.0) << "\n";
    std::cout << "Hazard (saturation 90%, vibration 18Hz) = "
              << A::computeHazardIndex(A::Saturation{90.0}, A::Vibration{18.0}) << "\n";
    std::cout << "Flash-flood (river 6.5m, rain 70, sat 80)= "
              << A::computeHazardIndex(6.5, 70.0, 80.0) << "\n";

    // call by reference + return by reference (chained in-place translation)
    S::GeoCoordinate origin{30.7346, 79.0669, 1890.0};
    S::GeoCoordinate probe = origin;
    S::translate(S::translate(probe, 0.010, 0.020), -0.002, 0.001);   // returns reference
    double x = 0, y = 0;
    S::toLocalGrid(origin, probe, x, y);                              // output by reference
    std::cout << "Probe moved to (" << probe.lat << ", " << probe.lon << ") -> local grid x="
              << x << " m, y=" << y << " m\n";

    S::TelemetryState st;
    S::accumulate(S::accumulate(S::accumulate(st, 42.0), 57.5), 63.0);  // state accumulated by ref
    std::cout << "Accumulated state: n=" << st.n << " sum=" << st.sum << " peak=" << st.peak << "\n";

    // friend class: audit private raw metrics without any getters
    S::SensorStation s(1, "Kedarghati", origin);
    s.update(85, 40, 90, 18);
    S::TelemetryAuditor auditor;
    auditor.audit(s, std::cout);
}

// =====================================================================
// UNIT II : ADTs, static members, dynamic memory, operator overloading
// =====================================================================
static void unitTwo() {
    banner("UNIT II: Encapsulation, dynamic memory & operator overloading");

    // --- dynamic memory: parameterized ctor, dynamic ctor, deep copy ---
    S::TelemetryBuffer b1(4);
    b1.push(10); b1.push(20); b1.push(30);
    S::TelemetryBuffer b2(b1);                  // deep copy
    b1.at(0) = 999;                             // return-by-reference write
    std::cout << "After b1.at(0)=999 : b1[0]=" << b1.at(0) << "  b2[0]=" << b2.at(0)
              << "  (deep copy => independent)\n";
    double raw[] = {1, 2, 3, 4, 5};
    S::TelemetryBuffer b3(raw, 5);              // dynamic constructor
    raw[0] = -1;
    std::cout << "Dynamic ctor copy: b3[0]=" << b3.at(0) << " size=" << b3.size() << "\n";
    b2 = b3;                                    // deep copy assignment
    std::cout << "b2 after assignment: size=" << b2.size() << " mean=" << b2.mean() << "\n";

    // --- static members: network-wide station count ---
    std::cout << "Active stations before block: " << S::SensorStation::activeStations() << "\n";
    {
        S::SensorStation a(1, "Kedarghati",  S::GeoCoordinate{30.73, 79.06, 1890});
        S::SensorStation b(2, "Rishikesh",   S::GeoCoordinate{30.09, 78.27, 372});
        S::SensorStation c(3, "Chamoli",     S::GeoCoordinate{30.40, 79.32, 1130});
        a.update(85, 40, 90, 18);
        b.update(30, 15, 45, 4);
        c.update(65, 35, 78, 11);
        std::cout << "Active stations after creating 3: " << S::SensorStation::activeStations() << "\n";
        { S::SensorStation tmp(a);
          std::cout << "Active stations with a copy alive: " << S::SensorStation::activeStations() << "\n"; }
        std::cout << "Active stations after copy destroyed: " << S::SensorStation::activeStations() << "\n";

        // --- operator overloading ---
        S::RegionalZone zone = a + b;                    // binary +
        zone = zone + c;                                 // chained
        std::cout << "Regional aggregate: " << zone << "\n";
        std::cout << "a > b ? " << (a > b ? "yes" : "no") << "   (hazard "
                  << a.hazard() << " vs " << b.hazard() << ")\n";
        const S::SensorStation& ro = a;                  // const correctness
        std::cout << "Const view: " << ro.name() << " hazard=" << ro.hazard()
                  << " history mean=" << ro.history().mean() << "\n";

        // << / >> on console and file streams
        std::cout << "Console <<: " << a << "\n";
        {
            std::ofstream out("zone_stations.txt");
            out << a << '\n' << b << '\n' << c << '\n';
        }
        std::ifstream in("zone_stations.txt");
        S::SensorStation r;
        std::cout << "Re-read via >>:\n";
        while (in >> r) std::cout << "   " << r << "\n";

        // --- static state in sirens ---
        A::EarlyWarningSiren::resetGlobal();
        A::EarlyWarningSiren s1(1, "Kedarghati"), s2(2, "Rishikesh");
        s1.trigger(A::classify(a.hazard()));
        s2.trigger(A::classify(b.hazard()));
        std::cout << "Siren1=" << A::toString(s1.level()) << " Siren2=" << A::toString(s2.level())
                  << " | global level=" << A::toString(A::EarlyWarningSiren::globalLevel())
                  << " activations=" << A::EarlyWarningSiren::totalActivations() << "\n";
    }
    std::cout << "Active stations after block: " << S::SensorStation::activeStations() << "\n";
}

// =====================================================================
// UNIT III : abstract classes, diamond, containership, runtime dispatch
// =====================================================================
static void unitThree() {
    banner("UNIT III: Inheritance, containership & polymorphism");
    S::GeoCoordinate g{30.5, 79.0, 1500};

    std::vector<S::BaseSensorNode*> nodes;
    nodes.push_back(new S::SoilMoistureNode(101, "Soil-Ridge", g, 76.0));
    nodes.push_back(new S::SeismicGeophone(102, "Geo-Fault", g, 11.0));
    nodes.push_back(new S::AcousticTiltmeter(103, "Tilt-Slope", g, 3.5));
    nodes.push_back(new S::RiverSonarGauge(104, "Sonar-Gauge", g, 3.8));
    nodes.push_back(new S::AutonomousHybridProbe(105, "Hybrid-Probe", g, 72.0, 12.0));

    for (int cycle = 1; cycle <= 3; ++cycle) {
        std::cout << "\n-- sweep cycle " << cycle << " --\n";
        if (cycle == 3) {                       // simulate a storm front
            nodes[0]->injectFault(86.0);
            nodes[3]->injectFault(5.4);
        }
        for (S::BaseSensorNode* n : nodes) {    // virtual dispatch through base pointers
            try {
                double v = n->readTelemetry();
                std::cout << A::col(14) << n->label() << A::col(18) << n->kind()
                          << A::col(9) << v << A::col(8) << (n->evaluateThreshold() ? "ALARM" : "ok")
                          << "battery " << n->power().batteryPct() << "%\n";
            } catch (const std::exception& e) {
                std::cout << n->label() << " -> " << e.what() << "\n";
            }
        }
    }

    // diamond: one shared DisasterTelemetryDevice / BaseSensorNode subobject
    S::AutonomousHybridProbe* probe = dynamic_cast<S::AutonomousHybridProbe*>(nodes.back());
    S::SolarPoweredNode*          sp = probe;
    S::WirelessLoRaTransceiver*   lo = probe;
    S::BaseSensorNode* viaSolar = sp;
    S::BaseSensorNode* viaLora  = lo;
    std::cout << "\nDiamond check: same BaseSensorNode subobject via both paths? "
              << (viaSolar == viaLora ? "YES (virtual base shared)" : "NO") << "\n";
    S::DisasterTelemetryDevice* dev = probe;
    std::cout << "Unambiguous deviceId(): " << probe->deviceId() << " | " << dev->describe() << "\n";
    std::cout << "Containership: probe owns PowerSubsystem, battery=" << probe->power().batteryPct()
              << "% (composed object, not inherited)\n";

    for (S::BaseSensorNode* n : nodes) delete n;   // virtual destructors => no leaks
    nodes.clear();
}

// =====================================================================
// UNIT IV : exceptions, binary persistence + rollback, STL, functors, manipulators
// =====================================================================
static std::streamoff fileSize(const std::string& p) {
    std::ifstream f(p, std::ios::binary | std::ios::ate);
    return f.tellg();
}

static void unitFour() {
    banner("UNIT IV: Streams, persistence, exception safety & STL");
    S::GeoCoordinate g{30.5, 79.0, 1500};

    // ---- custom exceptions ----
    std::cout << "[Exceptions]\n";
    try { S::SoilMoistureNode weak(201, "Weak-Node", g, 50.0, 1.0); weak.readTelemetry(); }
    catch (const SensorFailureException& e) { std::cout << "  caught: " << e.what() << "\n"; }
    try { S::RiverSonarGauge r(202, "Overflow-Gauge", g, 3.0); r.injectFault(55.0); r.readTelemetry(); }
    catch (const ThresholdOverflowException& e) { std::cout << "  caught: " << e.what() << "\n"; }
    try {
        S::TelemetryPacket p{105, 61.3, S::packetChecksum(105, 61.3)};
        p.value = 99.9;                                  // corruption in transit
        S::verifyPacket(p);
    } catch (const std::exception& e) { std::cout << "  caught (as std::exception): " << e.what() << "\n"; }

    // ---- binary persistence with transactional rollback ----
    const std::string path = "incident_log.bin";
    std::remove(path.c_str());
    {
        A::IncidentLogger log(path);
        std::cout << "\n[Binary log] sizeof(record)=" << sizeof(A::IncidentRecord) << " bytes\n";

        log.begin();                                         // transaction 1: commit
        log.append(A::makeRecord(1, 3, 88.2, 1001, "Kedarghati"));
        log.append(A::makeRecord(2, 2, 61.0, 1002, "Rishikesh"));
        log.append(A::makeRecord(3, 3, 79.5, 1003, "Chamoli"));
        log.commit();
        std::cout << "  after TX1 commit  : committed=" << log.count() << " file=" << fileSize(path) << " B\n";

        log.begin();                                         // transaction 2: abort
        try {
            log.append(A::makeRecord(4, 1, 40.3, 1004, "Joshimath"));
            log.append(A::makeRecord(5, 2, 58.9, 1005, "Kedarghati"));
            throw ThresholdOverflowException(250.0, 200.0);  // failure mid-transaction
        } catch (const std::exception& e) {
            std::cout << "  TX2 failed: " << e.what() << "\n";
            log.rollback();
        }
        std::cout << "  after TX2 rollback: committed=" << log.count() << " file=" << fileSize(path)
                  << " B (orphan bytes invisible)\n";

        log.begin();                                         // transaction 3: commit (overwrites orphans)
        log.append(A::makeRecord(4, 1, 40.3, 1004, "Joshimath"));
        log.append(A::makeRecord(5, 2, 58.9, 1005, "Kedarghati"));
        log.commit();
        std::cout << "  after TX3 commit  : committed=" << log.count() << " file=" << fileSize(path) << " B\n";
    }
    A::IncidentLogger reopened(path);                        // persistence across "restarts"
    std::vector<A::IncidentRecord> all = reopened.readAll(); // seekg + checksum verification
    std::cout << "  reopened log, read back " << all.size() << " verified records\n";

    // ---- STL: map, sort, for_each, functors, manipulators ----
    std::map<std::string, std::vector<A::IncidentRecord> > byRegion;
    for (std::vector<A::IncidentRecord>::const_iterator it = all.begin(); it != all.end(); ++it)
        byRegion[it->region].push_back(*it);
    std::cout << "\n[std::map grouping]\n";
    for (std::map<std::string, std::vector<A::IncidentRecord> >::const_iterator it = byRegion.begin();
         it != byRegion.end(); ++it)
        std::cout << "  " << it->first << ": " << it->second.size() << " incident(s)\n";

    std::map<std::string, int> population;
    population["Kedarghati"] = 4000;  population["Rishikesh"] = 120000;
    population["Chamoli"]    = 30000; population["Joshimath"] = 17000;

    A::EvacuationPriority prio(population);
    std::sort(all.begin(), all.end(), prio);                 // functor as comparator
    std::cout << "\n[Evacuation routing order]\n" << A::alertHeader;
    std::for_each(all.begin(), all.end(), A::DispatchPrinter(std::cout, prio));
    std::cout << A::tableRule;
}

int main() {
    std::cout << "ALF-EWG: Autonomous Landslide & Flash-Flood Early Warning Grid\n";
    S::seedSimulation(42);
    unitOne();
    unitTwo();
    unitThree();
    unitFour();
    std::cout << "\nAll four units demonstrated successfully.\n";
    return 0;
}
