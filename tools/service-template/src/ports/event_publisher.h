// Port: outbound event publication. The app layer depends on this interface;
// real services implement it against NATS JetStream (M1.4), the template against stdout.
//
// @req: UAOP-TPL-000  Template placeholder

#ifndef UAOP_TEMPLATE_PORTS_EVENT_PUBLISHER_H
#define UAOP_TEMPLATE_PORTS_EVENT_PUBLISHER_H

#include <string_view>

#include <uaop/common/error.h>
#include <uaop/common/result.h>

namespace uaop::tpl::ports {

class EventPublisher {
public:
    EventPublisher() = default;
    EventPublisher(const EventPublisher&) = delete;
    EventPublisher& operator=(const EventPublisher&) = delete;
    virtual ~EventPublisher() = default;

    [[nodiscard]] virtual common::Result<common::Unit, common::Error>
    publish(std::string_view subject, std::string_view payload) = 0;
};

} // namespace uaop::tpl::ports

#endif // UAOP_TEMPLATE_PORTS_EVENT_PUBLISHER_H
