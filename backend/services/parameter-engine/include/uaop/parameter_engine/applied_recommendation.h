// parameter-engine's side of the tune-fly-verify loop (ADR-0022): when a write that
// applies a FlightMD recommendation has been ACKed and read back from the FC, the
// param.written audit event carries the recommendation it came from, and
// flight-log-engine's analysis sidecar reports the change to FlightMD.
//
// Interface only. parameter-engine itself is M4.4; until then nothing implements
// or calls this. TODO(M4.4): the write path publishes through ParamWrittenPublisher
// after the ACK and read-back, copying WriteParameterRequest.recommendation; the
// @req annotations (UAOP-HLR-020/021) go on that implementation, not here.

#ifndef UAOP_PARAMETER_ENGINE_APPLIED_RECOMMENDATION_H
#define UAOP_PARAMETER_ENGINE_APPLIED_RECOMMENDATION_H

#include <string>
#include <string_view>

#include <uaop/common/error.h>
#include <uaop/common/result.h>

namespace uaop::parameter_engine {

// Subject and envelope schema of the event (EVENT_FLOW.md §5, events/v1/param.proto).
inline constexpr std::string_view kParamWrittenSubject = "uaop.event.v1.param.written";
inline constexpr std::string_view kParamWrittenSchema = "uaop.event.param.written/1";

// Mirrors gateway.v1.FlightmdRecommendation on WriteParameterRequest.
struct FlightmdRecommendation {
    std::string report_id;
    std::string airframe_label;
    std::string finding_id;

    [[nodiscard]] bool present() const noexcept { return !report_id.empty() && !airframe_label.empty(); }
};

// Mirrors events.v1.ParamWritten.
struct ParamWritten {
    std::string vehicle_id;
    std::string name;
    double old_value = 0.0; // int32 params fit a double exactly
    double new_value = 0.0;
    std::string operator_id;
    std::string risk_class; // LOW | MEDIUM | HIGH
    FlightmdRecommendation recommendation;
};

class ParamWrittenPublisher {
public:
    ParamWrittenPublisher() = default;
    ParamWrittenPublisher(const ParamWrittenPublisher&) = delete;
    ParamWrittenPublisher& operator=(const ParamWrittenPublisher&) = delete;
    virtual ~ParamWrittenPublisher() = default;

    // TRANSIENT when the event could not be durably persisted: the caller reports
    // the write as unaudited rather than dropping the event.
    [[nodiscard]] virtual common::Result<common::Unit, common::Error> publish(const ParamWritten& event) = 0;
};

} // namespace uaop::parameter_engine

#endif // UAOP_PARAMETER_ENGINE_APPLIED_RECOMMENDATION_H
