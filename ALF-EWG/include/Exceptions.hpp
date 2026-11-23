// Unit IV: custom exception hierarchy derived from std::exception
#pragma once
#include <cstdint>
#include <exception>
#include <string>

namespace DisasterCore {

class SensorFailureException : public std::exception {
    std::string msg_;
public:
    SensorFailureException(int nodeId, const std::string& reason)
        : msg_("SensorFailureException: node " + std::to_string(nodeId) +
               " failed -> " + reason) {}
    const char* what() const noexcept override { return msg_.c_str(); }
};

class TelemetryChecksumMismatch : public std::exception {
    std::string msg_;
    static std::string hex(std::uint32_t v) {
        static const char* d = "0123456789ABCDEF";
        std::string s(8, '0');
        for (int i = 7; i >= 0; --i, v >>= 4) s[i] = d[v & 0xF];
        return s;
    }
public:
    TelemetryChecksumMismatch(std::uint32_t expected, std::uint32_t actual)
        : msg_("TelemetryChecksumMismatch: stored 0x" + hex(expected) +
               " but computed 0x" + hex(actual)) {}
    const char* what() const noexcept override { return msg_.c_str(); }
};

class ThresholdOverflowException : public std::exception {
    std::string msg_;
public:
    ThresholdOverflowException(double value, double physicalLimit)
        : msg_("ThresholdOverflowException: reading " + std::to_string(value) +
               " exceeds physical sensor limit " + std::to_string(physicalLimit)) {}
    const char* what() const noexcept override { return msg_.c_str(); }
};

} // namespace DisasterCore
