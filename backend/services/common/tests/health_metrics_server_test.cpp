// @test: UAOP-TC-NFR008-G  HealthMetricsServer serves real GET /healthz and GET /metrics
// over a real socket (port 0 = OS-assigned, to avoid collisions on shared CI runners).

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <uaop/common/health.h>
#include <uaop/common/health_metrics_server.h>
#include <uaop/common/metrics.h>

namespace {

using uaop::common::HealthMetricsServer;
using uaop::common::HealthRegistry;
using uaop::common::HealthState;
using uaop::common::MetricRegistry;

#if defined(_WIN32)
using socket_t = SOCKET;
#else
using socket_t = int;
#endif

std::string http_get(uint16_t port, const std::string& path) {
    socket_t fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
#if defined(_WIN32)
        closesocket(fd);
#else
        ::close(fd);
#endif
        return "";
    }

    const std::string request = "GET " + path + " HTTP/1.1\r\nHost: localhost\r\n\r\n";
#if defined(_WIN32)
    send(fd, request.data(), static_cast<int>(request.size()), 0);
#else
    send(fd, request.data(), request.size(), 0);
#endif

    std::string response;
    char buf[4096];
    for (;;) {
#if defined(_WIN32)
        const int n = recv(fd, buf, sizeof(buf), 0);
#else
        const ssize_t n = recv(fd, buf, sizeof(buf), 0);
#endif
        if (n <= 0) {
            break;
        }
        response.append(buf, static_cast<std::size_t>(n));
    }
#if defined(_WIN32)
    closesocket(fd);
#else
    ::close(fd);
#endif
    return response;
}

void test_healthz_and_metrics_and_404() {
#if defined(_WIN32)
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif

    HealthRegistry health;
    health.set("nats", HealthState::kOk);
    MetricRegistry metrics;
    metrics.counter("frames_total", "Total frames")->increment(7);

    HealthMetricsServer server(health, metrics);
    assert(server.start(0)); // port 0 -> OS-assigned free port
    assert(server.port() != 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // let the accept loop start

    const std::string health_resp = http_get(server.port(), "/healthz");
    assert(health_resp.find("200 OK") != std::string::npos);
    assert(health_resp.find("\"status\":\"OK\"") != std::string::npos);

    const std::string metrics_resp = http_get(server.port(), "/metrics");
    assert(metrics_resp.find("200 OK") != std::string::npos);
    assert(metrics_resp.find("frames_total") != std::string::npos);
    assert(metrics_resp.find("7") != std::string::npos);

    const std::string missing_resp = http_get(server.port(), "/nonexistent");
    assert(missing_resp.find("404") != std::string::npos);

    server.stop();

#if defined(_WIN32)
    WSACleanup();
#endif
}

} // namespace

int main() {
    test_healthz_and_metrics_and_404();
    std::cout << "health-metrics-server-tests: all assertions passed\n";
    return EXIT_SUCCESS;
}
