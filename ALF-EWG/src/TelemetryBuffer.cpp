#include "TelemetryBuffer.hpp"
#include <stdexcept>

namespace DisasterCore {
namespace Sensors {

TelemetryBuffer::TelemetryBuffer(std::size_t capacity)
    : data_(new double[capacity ? capacity : 1]()),
      capacity_(capacity ? capacity : 1), count_(0), head_(0) {}

TelemetryBuffer::TelemetryBuffer(const double* src, std::size_t n)
    : data_(new double[n ? n : 1]()), capacity_(n ? n : 1), count_(n), head_(0) {
    for (std::size_t i = 0; i < n; ++i) data_[i] = src[i];
}

TelemetryBuffer::TelemetryBuffer(const TelemetryBuffer& o)
    : data_(new double[o.capacity_]), capacity_(o.capacity_),
      count_(o.count_), head_(o.head_) {
    for (std::size_t i = 0; i < capacity_; ++i) data_[i] = o.data_[i];
}

TelemetryBuffer& TelemetryBuffer::operator=(const TelemetryBuffer& o) {
    if (this == &o) return *this;
    double* fresh = new double[o.capacity_];          // allocate first: exception safe
    for (std::size_t i = 0; i < o.capacity_; ++i) fresh[i] = o.data_[i];
    delete[] data_;
    data_ = fresh; capacity_ = o.capacity_; count_ = o.count_; head_ = o.head_;
    return *this;
}

TelemetryBuffer::~TelemetryBuffer() { delete[] data_; }

std::size_t TelemetryBuffer::slot(std::size_t i) const {
    return (head_ + capacity_ - count_ + i) % capacity_;
}

void TelemetryBuffer::push(double v) {
    data_[head_] = v;
    head_ = (head_ + 1) % capacity_;
    if (count_ < capacity_) ++count_;
}

double& TelemetryBuffer::at(std::size_t i) {
    if (i >= count_) throw std::out_of_range("TelemetryBuffer::at index out of range");
    return data_[slot(i)];
}
const double& TelemetryBuffer::at(std::size_t i) const {
    if (i >= count_) throw std::out_of_range("TelemetryBuffer::at index out of range");
    return data_[slot(i)];
}

double TelemetryBuffer::latest() const {
    if (count_ == 0) throw std::out_of_range("TelemetryBuffer is empty");
    return data_[slot(count_ - 1)];
}

double TelemetryBuffer::mean() const {
    if (count_ == 0) return 0.0;
    double s = 0;
    for (std::size_t i = 0; i < count_; ++i) s += data_[slot(i)];
    return s / static_cast<double>(count_);
}

} // namespace Sensors
} // namespace DisasterCore
