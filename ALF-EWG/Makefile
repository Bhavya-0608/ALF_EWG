CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic -O2 -Iinclude
SRC      := $(wildcard src/*.cpp)
OBJ      := $(SRC:src/%.cpp=build/%.o)

alf_ewg: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build:
	mkdir -p build

run: alf_ewg
	./alf_ewg

clean:
	rm -rf build alf_ewg incident_log.bin zone_stations.txt
