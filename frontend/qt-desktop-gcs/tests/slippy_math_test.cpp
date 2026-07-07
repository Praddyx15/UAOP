// Pure-function tests for tile math — no Qt dependency needed.
// GCS is DAL D-equivalent (COMPLIANCE.md §A.2); no RTM/@req trace obligation applies here.

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>

#include "map/SlippyMath.h"

namespace {

using namespace uaop::gcs::map;

bool nearlyEqual(double a, double b, double eps = 1e-6) { return std::abs(a - b) < eps; }

void test_tiles_per_axis() {
    assert(tilesPerAxis(0) == 1);
    assert(tilesPerAxis(3) == 8);
    assert(tilesPerAxis(10) == 1024);
}

void test_known_tile_zero_zero_is_top_left() {
    // (-180, 85.0511) is the top-left corner of the Web Mercator world at any zoom.
    const auto frac = lonLatToTileFrac(-180.0, 85.0511, 2);
    assert(nearlyEqual(frac.x, 0.0, 1e-3));
    assert(nearlyEqual(frac.y, 0.0, 1e-3));
}

void test_center_of_world_is_center_tile() {
    const int z = 4;
    const auto frac = lonLatToTileFrac(0.0, 0.0, z);
    const double n = static_cast<double>(tilesPerAxis(z));
    assert(nearlyEqual(frac.x, n / 2.0, 1e-6));
    assert(nearlyEqual(frac.y, n / 2.0, 1e-6));
}

void test_roundtrip_lonlat_tile_lonlat() {
    const int z = 6;
    const double lon0 = 77.0266, lat0 = 28.4595; // Gurugram
    const auto frac = lonLatToTileFrac(lon0, lat0, z);
    double lon1 = 0, lat1 = 0;
    tileFracToLonLat(frac.x, frac.y, z, lon1, lat1);
    assert(nearlyEqual(lon0, lon1, 1e-6));
    assert(nearlyEqual(lat0, lat1, 1e-6));
}

void test_xyz_to_tms_row_flip() {
    assert(xyzToTmsRow(0, 3) == 7);  // top row (XYZ) is last row (TMS)
    assert(xyzToTmsRow(7, 3) == 0);
    assert(xyzToTmsRow(3, 3) == 4);  // self-inverse at the midpoint pairing
}

} // namespace

int main() {
    test_tiles_per_axis();
    test_known_tile_zero_zero_is_top_left();
    test_center_of_world_is_center_tile();
    test_roundtrip_lonlat_tile_lonlat();
    test_xyz_to_tms_row_flip();
    std::cout << "slippy_math_test: all assertions passed\n";
    return EXIT_SUCCESS;
}
