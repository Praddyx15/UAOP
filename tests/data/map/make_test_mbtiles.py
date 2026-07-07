#!/usr/bin/env python3
"""Generates world.mbtiles — a tiny synthetic raster tile set (zoom 0-3) for
offline map-path testing. Standard MBTiles 1.3 (SQLite, TMS row order, PNG
tiles). Stdlib only: PNGs are hand-encoded (zlib), no imaging dependency.

Tiles are flat-colored by position with a border and center cross so panning,
zooming, and tile addressing are visually verifiable. Not cartography — the
real basemap packages come from Planetiler/QGIS exports (ADR-0020).
"""

import sqlite3
import struct
import zlib
from pathlib import Path

SIZE = 256
MAX_ZOOM = 3
OUT = Path(__file__).parent / "world.mbtiles"


def png_chunk(kind: bytes, data: bytes) -> bytes:
    return (struct.pack(">I", len(data)) + kind + data +
            struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF))


def make_tile_png(z: int, x: int, y: int) -> bytes:
    # Position-derived flat color, dark aerospace-ish range
    r = 24 + (x * 67) % 90
    g = 34 + (y * 53) % 90
    b = 44 + (z * 41) % 120
    border = (90, 105, 120)
    cross = (60, 72, 84)

    rows = bytearray()
    for py in range(SIZE):
        rows.append(0)  # filter: none
        for px in range(SIZE):
            on_border = px < 2 or py < 2 or px >= SIZE - 2 or py >= SIZE - 2
            on_cross = abs(px - SIZE // 2) < 1 or abs(py - SIZE // 2) < 1
            c = border if on_border else (cross if on_cross else (r, g, b))
            rows.extend(c)

    ihdr = struct.pack(">IIBBBBB", SIZE, SIZE, 8, 2, 0, 0, 0)  # 8-bit RGB
    return (b"\x89PNG\r\n\x1a\n" + png_chunk(b"IHDR", ihdr) +
            png_chunk(b"IDAT", zlib.compress(bytes(rows), 9)) +
            png_chunk(b"IEND", b""))


def main() -> None:
    OUT.unlink(missing_ok=True)
    db = sqlite3.connect(OUT)
    db.executescript("""
        CREATE TABLE metadata (name TEXT, value TEXT);
        CREATE TABLE tiles (zoom_level INTEGER, tile_column INTEGER,
                            tile_row INTEGER, tile_data BLOB);
        CREATE UNIQUE INDEX tile_index ON tiles (zoom_level, tile_column, tile_row);
    """)
    db.executemany("INSERT INTO metadata VALUES (?, ?)", [
        ("name", "UAOP synthetic test tiles"),
        ("format", "png"),
        ("minzoom", "0"),
        ("maxzoom", str(MAX_ZOOM)),
        ("bounds", "-180.0,-85.0511,180.0,85.0511"),
        ("type", "baselayer"),
        ("version", "1"),
    ])
    count = 0
    for z in range(MAX_ZOOM + 1):
        n = 1 << z
        for x in range(n):
            for y in range(n):
                tms_row = n - 1 - y  # MBTiles stores TMS row order
                db.execute("INSERT INTO tiles VALUES (?, ?, ?, ?)",
                           (z, x, tms_row, make_tile_png(z, x, y)))
                count += 1
    db.commit()
    db.close()
    print(f"wrote {OUT.name}: {count} tiles, {OUT.stat().st_size / 1024:.1f} KB")


if __name__ == "__main__":
    main()
