// Adapter: EventPublisher against stdout. Real services swap this file for a NATS
// JetStream client (M1.4) without the domain or app layers noticing — that is the point.
//
// @req: UAOP-TPL-000  Template placeholder

#ifndef UAOP_TEMPLATE_ADAPTERS_STDOUT_PUBLISHER_H
#define UAOP_TEMPLATE_ADAPTERS_STDOUT_PUBLISHER_H

#include <iostream>

#include "../ports/event_publisher.h"

namespace uaop::tpl::adapters {

class StdoutPublisher final : public ports::EventPublisher {
public:
    [[nodiscard]] common::Result<common::Unit, common::Error>
    publish(std::string_view subject, std::string_view payload) override {
        std::cout << subject << ' ' << payload << '\n';
        return common::Result<common::Unit, common::Error>::Ok(common::kUnit);
    }
};

} // namespace uaop::tpl::adapters

#endif // UAOP_TEMPLATE_ADAPTERS_STDOUT_PUBLISHER_H
