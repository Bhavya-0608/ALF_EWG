// Unit II: dynamic memory - parameterized/dynamic ctor, deep copy, destructor
#pragma once
#include <cstddef>

namespace DisasterCore {
namespace Sensors {

class TelemetryBuffer {
    double*     data_;
    std::size_t capacity_;
    std::size_t count_;
    std::size_t head_;                     // next write slot (circular)
    std::size_t slot(std::size_t i) const; // logical index (0 = oldest) -> physical
public:
    explicit TelemetryBuffer(std::size_t capacity = 8);        // parameterized ctor
    TelemetryBuffer(const double* src, std::size_t n);         // dynamic ctor (allocates + copies)
    TelemetryBuffer(const TelemetryBuffer& other);             // deep copy ctor
    TelemetryBuffer& operator=(const TelemetryBuffer& other);  // deep copy assignment
    ~TelemetryBuffer();                                        // explicit destructor

    void push(double v);                    // overwrites oldest when full
    double&       at(std::size_t i);        // return by reference (throws std::out_of_range)
    const double& at(std::size_t i) const;
    double latest() const;
    double mean() const;
    std::size_t size() const     { return count_; }
    std::size_t capacity() const { return capacity_; }
};

} // namespace Sensors
} // namespace DisasterCore
