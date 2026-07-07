// Slippy-map tile math (OSM/TMS conventions) — pure functions, no Qt/DB
// dependency, unit-testable in isolation (matches the hexagonal template's
// domain-layer discipline even though the GCS itself is DAL D-equivalent).

#ifndef UAOP_GCS_MAP_SLIPPYMATH_H
#define UAOP_GCS_MAP_SLIPPYMATH_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numbers>

namespace uaop::gcs::map {

inline constexpr double kPi = std::numbers::pi_v<double>;

struct TileId {
    int z = 0;
    std::int64_t x = 0;
    std::int64_t y = 0; // XYZ convention (top-left origin); flip to TMS at the DB boundary

    bool operator==(const TileId&) const = default;
};

struct PixelPoint {
    double x = 0.0;
    double y = 0.0;
};

// Web Mercator tile count per axis at zoom z.
inline std::int64_t tilesPerAxis(int z) noexcept {
    return std::int64_t{1} << std::clamp(z, 0, 22);
}

// lon/lat (degrees, WGS84) -> fractional tile coordinates at zoom z (XYZ).
inline PixelPoint lonLatToTileFrac(double lonDeg, double latDeg, int z) noexcept {
    const double n = static_cast<double>(tilesPerAxis(z));
    const double latRad = latDeg * kPi / 180.0;
    const double x = (lonDeg + 180.0) / 360.0 * n;
    const double y = (1.0 - std::asinh(std::tan(latRad)) / kPi) / 2.0 * n;
    return {x, y};
}

// Inverse: tile-space fractional coords at zoom z -> lon/lat (degrees).
inline void tileFracToLonLat(double xFrac, double yFrac, int z, double& lonDeg,
                             double& latDeg) noexcept {
    const double n = static_cast<double>(tilesPerAxis(z));
    lonDeg = xFrac / n * 360.0 - 180.0;
    const double latRad = std::atan(std::sinh(kPi * (1.0 - 2.0 * yFrac / n)));
    latDeg = latRad * 180.0 / kPi;
}

// XYZ row -> TMS row (the on-disk convention in MBTiles), per zoom.
inline std::int64_t xyzToTmsRow(std::int64_t xyzY, int z) noexcept {
    return tilesPerAxis(z) - 1 - xyzY;
}

} // namespace uaop::gcs::map

#endif // UAOP_GCS_MAP_SLIPPYMATH_H
