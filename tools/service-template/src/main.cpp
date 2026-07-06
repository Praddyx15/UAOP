// main.cpp is wiring only: adapters -> app -> run. No logic lives here.
// Simulates a link that heartbeats, then goes silent past the timeout.

#include <cstdlib>
#include <iostream>

#include "adapters/stdout_publisher.h"
#include "app/supervise_link.h"
#include "domain/link_monitor.h"

int main() {
    constexpr std::uint64_t kTimeoutMs = 3000; // UAOP-HLR-004-style 3 s heartbeat timeout

    uaop::tpl::domain::LinkMonitor monitor(kTimeoutMs);
    uaop::tpl::adapters::StdoutPublisher publisher;
    uaop::tpl::app::SuperviseLink use_case(monitor, publisher);

    for (std::uint64_t t = 0; t <= 2000; t += 1000) {
        if (const auto r = use_case.heartbeat_received(t); !r.ok()) {
            std::cerr << "heartbeat error: " << r.error().message() << '\n';
            return EXIT_FAILURE;
        }
    }
    // Silence: ticks past the timeout must yield exactly one LOST transition.
    for (std::uint64_t t = 3000; t <= 7000; t += 1000) {
        if (const auto r = use_case.tick(t); !r.ok()) {
            std::cerr << "tick error: " << r.error().message() << '\n';
            return EXIT_FAILURE;
        }
    }
    std::cout << "service-template: done\n";
    return EXIT_SUCCESS;
}
