#include <uaop/common/metrics.h>

#include <cstring>
#include <sstream>

namespace uaop::common {

uint64_t Gauge::to_bits(double v) noexcept {
    static_assert(sizeof(double) == sizeof(uint64_t));
    uint64_t bits = 0;
    std::memcpy(&bits, &v, sizeof(bits));
    return bits;
}

double Gauge::from_bits(uint64_t b) noexcept {
    double v = 0.0;
    std::memcpy(&v, &b, sizeof(v));
    return v;
}

namespace {
std::string render_labels(const Labels& labels) {
    if (labels.empty()) {
        return "";
    }
    std::ostringstream oss;
    oss << '{';
    bool first = true;
    for (const auto& [k, v] : labels) {
        if (!first) {
            oss << ',';
        }
        first = false;
        oss << k << "=\"" << v << '"';
    }
    oss << '}';
    return oss.str();
}
} // namespace

std::string MetricRegistry::key(const std::string& name, const Labels& labels) const {
    return name + render_labels(labels);
}

std::shared_ptr<Counter> MetricRegistry::counter(const std::string& name, const std::string& help,
                                                  Labels labels) {
    std::lock_guard<std::mutex> lock(mutex_);
    const std::string k = key(name, labels);
    auto it = index_.find(k);
    if (it != index_.end()) {
        return entries_[it->second].counter;
    }
    MetricEntry entry;
    entry.name = name;
    entry.help = help;
    entry.labels = std::move(labels);
    entry.kind = MetricEntry::Kind::kCounter;
    entry.counter = std::make_shared<Counter>();
    index_[k] = entries_.size();
    entries_.push_back(entry);
    return entry.counter;
}

std::shared_ptr<Gauge> MetricRegistry::gauge(const std::string& name, const std::string& help,
                                              Labels labels) {
    std::lock_guard<std::mutex> lock(mutex_);
    const std::string k = key(name, labels);
    auto it = index_.find(k);
    if (it != index_.end()) {
        return entries_[it->second].gauge;
    }
    MetricEntry entry;
    entry.name = name;
    entry.help = help;
    entry.labels = std::move(labels);
    entry.kind = MetricEntry::Kind::kGauge;
    entry.gauge = std::make_shared<Gauge>();
    index_[k] = entries_.size();
    entries_.push_back(entry);
    return entry.gauge;
}

std::string MetricRegistry::render_prometheus() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream oss;
    std::string last_name;
    for (const auto& entry : entries_) {
        if (entry.name != last_name) {
            oss << "# HELP " << entry.name << ' ' << entry.help << '\n';
            oss << "# TYPE " << entry.name << ' '
                << (entry.kind == MetricEntry::Kind::kCounter ? "counter" : "gauge") << '\n';
            last_name = entry.name;
        }
        oss << entry.name << render_labels(entry.labels) << ' '
            << (entry.kind == MetricEntry::Kind::kCounter ? static_cast<double>(entry.counter->value())
                                                            : entry.gauge->value())
            << '\n';
    }
    return oss.str();
}

} // namespace uaop::common
