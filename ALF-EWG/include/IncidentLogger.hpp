// Unit IV: binary std::fstream persistence with transactional rollback via seekp()/seekg()
#pragma once
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace DisasterCore {
namespace Alerts {

// Fixed-layout POD record serialised to disk.
struct IncidentRecord {
    std::int32_t  stationId;
    std::int32_t  level;        // AlertLevel as int
    double        hazard;
    std::int64_t  timestamp;
    char          region[32];
    std::uint32_t checksum;
};

std::uint32_t recordChecksum(const IncidentRecord& r);
IncidentRecord makeRecord(int stationId, int level, double hazard,
                          std::int64_t ts, const std::string& region);

// File layout: [magic u32][committedCount u32][record]*  -- header is updated ONLY on commit,
// so records written inside an aborted transaction are invisible and later overwritten.
class IncidentLogger {
    std::string   path_;
    std::fstream  file_;
    std::uint32_t count_;      // committed records
    std::uint32_t pending_;    // records written in the open transaction
    bool          inTx_;

    static const std::uint32_t kMagic  = 0x414C4645u;   // 'ALFE'
    static const std::streamoff kHeader = 8;
    static std::streamoff offsetOf(std::uint32_t n);
    void writeHeader();
public:
    explicit IncidentLogger(const std::string& path);
    IncidentLogger(const IncidentLogger&) = delete;
    IncidentLogger& operator=(const IncidentLogger&) = delete;
    ~IncidentLogger();

    void begin();
    void append(const IncidentRecord& r);
    void commit();
    void rollback();
    bool inTransaction() const { return inTx_; }
    std::uint32_t count() const { return count_; }
    std::vector<IncidentRecord> readAll();   // verifies each checksum
};

} // namespace Alerts
} // namespace DisasterCore
