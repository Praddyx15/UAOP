# UAOP

The flight-evidence layer for PX4 and ArduPilot drones: capture every flight, keep a tamper-evident
record of it, and prove what happened, offline.

[![CI](https://github.com/Praddyx15/UAOP/actions/workflows/pr.yml/badge.svg?branch=main)](https://github.com/Praddyx15/UAOP/actions/workflows/pr.yml)
[![License: Apache-2.0](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](LICENSE)

UAOP (Unified Autonomy Operating Platform) runs on a laptop or an edge computer next to the
aircraft. It talks to the autopilot over MAVLink, records the flight, and logs every command and
parameter change in a record that shows if anything was altered. After the flight it brings
together the log, the parameters, the [FlightMD](https://github.com/Praddyx15/FlightMD) diagnosis
and the [AirBrief](https://github.com/Praddyx15/AirBrief) pre-flight brief, so anyone can check
what happened without an internet connection.

It sits above the autopilot and never replaces it: PX4 and ArduPilot keep flying the aircraft. It
can also sit beside QGroundControl or Mission Planner and only observe. It reports evidence; it
never says go or no-go.

> **Status: early development.** UAOP is a personal project. The foundations are built and tested;
> live telemetry is next. Nothing here is certified or ready to fly with.

## Status

| Milestone | What it delivers | State |
|---|---|---|
| M0 | Toolchain, CI on x86_64 and ARM64, desktop shell | Done |
| M1 | Message contracts, platform plumbing, storage | Done |
| M2 | First live telemetry from a simulated drone | Next |
| M2E | Flight evidence for PX4 and ArduPilot, observer mode | Planned |
| M3–M7 | Commands, missions and parameters, compliance, hardening | Planned |

## Works with FlightMD and AirBrief

- [**AirBrief**](https://github.com/Praddyx15/AirBrief) answers the questions before take-off:
  airspace, terrain, who is underneath, line of sight. UAOP keeps the brief with the flight.
- [**FlightMD**](https://github.com/Praddyx15/FlightMD) reads the log afterwards and says what went
  wrong and which parameter to change. UAOP runs it on every captured log and tells FlightMD when a
  recommended change was applied, so the next report shows whether it worked.

## Try it

You need Docker, CMake 3.25+, Ninja, a C++20 compiler, protobuf and Python 3.11+. The desktop app
also needs Qt 6.8 or later.

```bash
git clone https://github.com/Praddyx15/UAOP.git && cd UAOP
./tools/setup.sh up      # local services in Docker, then build, test and run the checks
./tools/setup.sh sitl    # optional: adds a PX4 simulator (MAVLink on udp:14550)
./tools/setup.sh down    # stop everything
```

Build and test by hand:

```bash
cmake --preset debug && cmake --build --preset debug && ctest --preset debug
cmake --preset gcs-debug -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64 \
  && cmake --build --preset gcs-debug && ctest --preset gcs-debug
```

## Contributing and security

[CONTRIBUTING.md](CONTRIBUTING.md) has the rules every change follows and where the design
documents live. [SECURITY.md](SECURITY.md) explains how to report a vulnerability.

## Licence

Apache-2.0 ([LICENSE](LICENSE), [NOTICE](NOTICE)). Bundled third-party material is listed in
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md), each under its own licence.

UAOP is development software. It is not certified, it is not an airworthiness or safety
determination, and it does not replace any authorisation a flight needs.
