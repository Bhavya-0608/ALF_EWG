#include "IncidentLogger.hpp"
#include "Exceptions.hpp"
#include <cstring>
#include <stdexcept>

namespace DisasterCore {
namespace Alerts {

std::uint32_t recordChecksum(const IncidentRecord& r) {
    std::uint32_t h = 2166136261u;
    auto mix = [&h](const void* p, std::size_t n) {
        const unsigned char* b = static_cast<const unsigned char*>(p);
        for (std::size_t i = 0; i < n; ++i) { h ^= b[i]; h *= 16777619u; }
    };
    mix(&r.stationId, sizeof r.stationId);
    mix(&r.level,     sizeof r.level);
    mix(&r.hazard,    sizeof r.hazard);
    mix(&r.timestamp, sizeof r.timestamp);
    mix(r.region,     sizeof r.region);
    return h;
}

IncidentRecord makeRecord(int stationId, int level, double hazard,
                          std::int64_t ts, const std::string& region) {
    IncidentRecord r;
    std::memset(&r, 0, sizeof r);
    r.stationId = stationId; r.level = level; r.hazard = hazard; r.timestamp = ts;
    std::strncpy(r.region, region.c_str(), sizeof r.region - 1);
    r.checksum = recordChecksum(r);
    return r;
}

std::streamoff IncidentLogger::offsetOf(std::uint32_t n) {
    return kHeader + static_cast<std::streamoff>(n) * static_cast<std::streamoff>(sizeof(IncidentRecord));
}

IncidentLogger::IncidentLogger(const std::string& path)
    : path_(path), count_(0), pending_(0), inTx_(false) {
    file_.open(path_, std::ios::in | std::ios::out | std::ios::binary);
    if (!file_.is_open()) {                       // file does not exist yet: create it
        file_.clear();
        file_.open(path_, std::ios::out | std::ios::binary);
        file_.close();
        file_.clear();
        file_.open(path_, std::ios::in | std::ios::out | std::ios::binary);
    }
    if (!file_.is_open()) throw std::runtime_error("cannot open log file: " + path_);

    file_.seekg(0, std::ios::end);
    if (file_.tellg() == std::streampos(0)) {
        writeHeader();
    } else {
        std::uint32_t magic = 0;
        file_.seekg(0, std::ios::beg);
        file_.read(reinterpret_cast<char*>(&magic), sizeof magic);
        file_.read(reinterpret_cast<char*>(&count_), sizeof count_);
        if (!file_ || magic != kMagic) throw std::runtime_error("not an ALF-EWG log file: " + path_);
    }
}

IncidentLogger::~IncidentLogger() {
    if (inTx_) { try { rollback(); } catch (...) {} }   // never leave a half transaction
}

void IncidentLogger::writeHeader() {
    std::uint32_t magic = kMagic;
    file_.clear();
    file_.seekp(0, std::ios::beg);
    file_.write(reinterpret_cast<const char*>(&magic), sizeof magic);
    file_.write(reinterpret_cast<const char*>(&count_), sizeof count_);
    file_.flush();
    if (!file_) throw std::runtime_error("failed to write log header");
}

void IncidentLogger::begin() {
    if (inTx_) throw std::logic_error("transaction already open");
    inTx_ = true; pending_ = 0;
    file_.clear();
    file_.seekp(offsetOf(count_), std::ios::beg);       // start of uncommitted area
}

void IncidentLogger::append(const IncidentRecord& r) {
    if (!inTx_) throw std::logic_error("append outside transaction");
    file_.clear();
    file_.seekp(offsetOf(count_ + pending_), std::ios::beg);
    file_.write(reinterpret_cast<const char*>(&r), sizeof r);
    file_.flush();
    if (!file_) throw std::runtime_error("disk write failed");
    ++pending_;
}

void IncidentLogger::commit() {
    if (!inTx_) throw std::logic_error("commit without begin");
    count_ += pending_;
    pending_ = 0;
    inTx_ = false;
    writeHeader();                                       // publish new committed count
}

void IncidentLogger::rollback() {
    if (!inTx_) throw std::logic_error("rollback without begin");
    pending_ = 0;
    inTx_ = false;
    file_.clear();
    file_.seekp(offsetOf(count_), std::ios::beg);        // rewind write head to last commit
}

std::vector<IncidentRecord> IncidentLogger::readAll() {
    std::vector<IncidentRecord> out;
    file_.clear();
    file_.seekg(kHeader, std::ios::beg);
    for (std::uint32_t i = 0; i < count_; ++i) {
        IncidentRecord r;
        file_.read(reinterpret_cast<char*>(&r), sizeof r);
        if (!file_) throw std::runtime_error("truncated log file");
        std::uint32_t actual = recordChecksum(r);
        if (actual != r.checksum) throw TelemetryChecksumMismatch(r.checksum, actual);
        out.push_back(r);
    }
    return out;
}

} // namespace Alerts
} // namespace DisasterCore
