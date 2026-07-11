#include <uaop/common/health_metrics_server.h>

#include <cstring>
#include <sstream>
#include <string>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#endif

namespace uaop::common {

namespace {

#if defined(_WIN32)
using socket_t = SOCKET;
constexpr socket_t kInvalidSocket = INVALID_SOCKET;
void close_socket(socket_t s) { closesocket(s); }
#else
using socket_t = int;
constexpr socket_t kInvalidSocket = -1;
void close_socket(socket_t s) { ::close(s); }
#endif

constexpr int kBacklog = 8;
constexpr int kPollTimeoutMs = 200;

std::string http_response(int status, const char* status_text, const std::string& content_type,
                           const std::string& body) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status << ' ' << status_text << "\r\n"
        << "Content-Type: " << content_type << "\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n\r\n"
        << body;
    return oss.str();
}

} // namespace

HealthMetricsServer::HealthMetricsServer(HealthRegistry& health, MetricRegistry& metrics)
    : health_(health), metrics_(metrics) {
#if defined(_WIN32)
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
}

HealthMetricsServer::~HealthMetricsServer() {
    stop();
#if defined(_WIN32)
    WSACleanup();
#endif
}

bool HealthMetricsServer::start(uint16_t port) {
    if (running_.load()) {
        return false; // already started
    }

    const socket_t fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == kInvalidSocket) {
        return false;
    }

    int reuse = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        close_socket(fd);
        return false;
    }
    if (listen(fd, kBacklog) != 0) {
        close_socket(fd);
        return false;
    }

    if (port == 0) {
        sockaddr_in bound{};
#if defined(_WIN32)
        int len = sizeof(bound);
#else
        socklen_t len = sizeof(bound);
#endif
        if (getsockname(fd, reinterpret_cast<sockaddr*>(&bound), &len) == 0) {
            port = ntohs(bound.sin_port);
        }
    }

    listen_fd_ = static_cast<int64_t>(fd);
    port_ = port;
    running_.store(true);
    accept_thread_ = std::thread(&HealthMetricsServer::run, this);
    return true;
}

void HealthMetricsServer::stop() {
    if (!running_.exchange(false)) {
        return;
    }
    if (listen_fd_ >= 0) {
        close_socket(static_cast<socket_t>(listen_fd_));
        listen_fd_ = -1;
    }
    if (accept_thread_.joinable()) {
        accept_thread_.join();
    }
}

void HealthMetricsServer::run() {
    const socket_t fd = static_cast<socket_t>(listen_fd_);
    while (running_.load()) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);
        timeval tv{};
        tv.tv_sec = 0;
        tv.tv_usec = kPollTimeoutMs * 1000;

        const int ready = select(static_cast<int>(fd) + 1, &read_fds, nullptr, nullptr, &tv);
        if (ready <= 0) {
            continue; // timeout (re-check running_) or interrupted
        }

        sockaddr_in client_addr{};
#if defined(_WIN32)
        int client_len = sizeof(client_addr);
#else
        socklen_t client_len = sizeof(client_addr);
#endif
        const socket_t client = accept(fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
        if (client == kInvalidSocket) {
            continue;
        }
        handle_connection(static_cast<int64_t>(client));
    }
}

void HealthMetricsServer::handle_connection(int64_t client_fd_raw) {
    const socket_t client = static_cast<socket_t>(client_fd_raw);

    char buf[2048];
#if defined(_WIN32)
    const int n = recv(client, buf, sizeof(buf) - 1, 0);
#else
    const ssize_t n = recv(client, buf, sizeof(buf) - 1, 0);
#endif
    if (n > 0) {
        buf[n] = '\0';
        const std::string request(buf);
        const std::string response = (request.rfind("GET /healthz", 0) == 0)
            ? http_response(200, "OK", "application/json", health_.render_json())
            : (request.rfind("GET /metrics", 0) == 0)
                ? http_response(200, "OK", "text/plain; version=0.0.4", metrics_.render_prometheus())
                : http_response(404, "Not Found", "text/plain", "not found\n");
#if defined(_WIN32)
        send(client, response.data(), static_cast<int>(response.size()), 0);
#else
        send(client, response.data(), response.size(), 0);
#endif
    }
    close_socket(client);
}

} // namespace uaop::common
