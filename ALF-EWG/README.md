# ALF-EWG — Autonomous Landslide & Flash-Flood Early Warning Grid

**Course:** CIC-211 Object-Oriented Programming using C++ · **Project:** Option 1
**Standard:** C++11/14/17 compliant (tested with g++, `-Wall -Wextra -pedantic`, ASan + UBSan clean)

## Build & run
```bash
make          # builds ./alf_ewg
make run      # builds and runs the demo of all four units
make clean
```
Without make: `g++ -std=c++17 -Iinclude src/*.cpp -o alf_ewg && ./alf_ewg`

## Project layout
```
include/   Exceptions.hpp  HazardIndex.hpp  Geo.hpp  TelemetryBuffer.hpp
           SensorStation.hpp  Siren.hpp  SensorNodes.hpp
           IncidentLogger.hpp  Reporting.hpp
src/       TelemetryBuffer.cpp  SensorStation.cpp  Siren.cpp  SensorNodes.cpp
           IncidentLogger.cpp  main.cpp
docs/      sample_output.txt
```

## Syllabus coverage map
| Requirement | Where |
|---|---|
| **Unit I** namespaces `DisasterCore::Sensors` / `::Alerts` | all headers |
| inline risk functions, overloaded `computeHazardIndex()` (3 overloads) | `HazardIndex.hpp` |
| call by reference / return by reference | `Geo.hpp` (`translate`, `toLocalGrid`, `accumulate`), `TelemetryBuffer::at()` |
| friend class `TelemetryAuditor` | `SensorStation.hpp/.cpp` |
| **Unit II** ADTs + const correctness | `SensorStation`, `EarlyWarningSiren`, `TelemetryBuffer` |
| static data/member functions | `SensorStation::activeStations()`, `EarlyWarningSiren::globalLevel()` |
| parameterized / dynamic / deep-copy ctors, destructor | `TelemetryBuffer.cpp` |
| operators `+`, `>`, `<<`, `>>` | `SensorStation.cpp`, `RegionalZone` |
| **Unit III** abstract `BaseSensorNode` (pure virtuals) | `SensorNodes.hpp` |
| diamond via virtual bases (`DisasterTelemetryDevice` → `SolarPoweredNode` / `WirelessLoRaTransceiver` → `AutonomousHybridProbe`) | `SensorNodes.hpp/.cpp` |
| containership: `PowerSubsystem` inside `BaseSensorNode` | `SensorNodes.hpp` |
| `std::vector<BaseSensorNode*>` dynamic dispatch | `main.cpp` `unitThree()` |
| **Unit IV** custom exceptions with `what()` | `Exceptions.hpp` |
| binary `std::fstream` log + transactional rollback (`seekp`/`seekg`) | `IncidentLogger.cpp` |
| STL `vector`, `map`, `sort`, `for_each`, functors, manipulators | `Reporting.hpp`, `main.cpp` `unitFour()` |

## Push to GitHub
```bash
git init && git add . && git commit -m "ALF-EWG: CIC-211 OOP project"
git branch -M main
git remote add origin https://github.com/<your-username>/ALF-EWG.git
git push -u origin main
```
