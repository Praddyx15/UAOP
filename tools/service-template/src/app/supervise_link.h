// App layer: the use case. Composes domain + ports; this is where a real service
// emits its audit events before reporting success (SOFTWARE_ARCHITECTURE.md §3).
//
// @req: UAOP-TPL-000  Template placeholder

#ifndef UAOP_TEMPLATE_APP_SUPERVISE_LINK_H
#define UAOP_TEMPLATE_APP_SUPERVISE_LINK_H

#include <cstdint>
#include <string>

#include "../domain/link_monitor.h"
#include "../ports/event_publisher.h"

namespace uaop::tpl::app {

class SuperviseLink {
public:
    SuperviseLink(domain::LinkMonitor& monitor, ports::EventPublisher& publisher)
        : monitor_(monitor), publisher_(publisher) {}

    [[nodiscard]] common::Result<common::Unit, common::Error>
    heartbeat_received(std::uint64_t now_ms) {
        auto result = monitor_.on_heartbeat(now_ms);
        if (!result.ok()) {
            return common::Result<common::Unit, common::Error>::Err(result.error());
        }
        return publish_if_changed(result.value());
    }

    [[nodiscard]] common::Result<common::Unit, common::Error> tick(std::uint64_t now_ms) {
        return publish_if_changed(monitor_.on_tick(now_ms));
    }

private:
    [[nodiscard]] common::Result<common::Unit, common::Error>
    publish_if_changed(const domain::Transition& t) {
        using R = common::Result<common::Unit, common::Error>;
        if (!t.changed) {
            return R::Ok(common::kUnit);
        }
        const std::string payload = std::string("{\"from\":") +
                                    std::to_string(static_cast<int>(t.from)) +
                                    ",\"to\":" + std::to_string(static_cast<int>(t.to)) + "}";
        return publisher_.publish("uaop.event.v1.tpl.link_state_changed", payload);
    }

    domain::LinkMonitor& monitor_;
    ports::EventPublisher& publisher_;
};

} // namespace uaop::tpl::app

#endif // UAOP_TEMPLATE_APP_SUPERVISE_LINK_H
