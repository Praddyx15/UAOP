# Security

## Reporting a problem

Please report it privately through **Security → Report a vulnerability** on this repository, not in
a public issue. Say what is affected, how to reproduce it and what an attacker could do. You'll hear
back within a week, and fixes ship as soon as they're ready (with credit if you like).

## Where things stand

UAOP is in early development and has not been security-assessed by anyone. Do not use it to fly a
real aircraft or to hold data you need to protect.

## How it is meant to run

On a workstation or an edge computer beside the aircraft, with no internet required. The operator
controls the machine and its keys; everything arriving from the aircraft link, the API or an
imported file is treated as untrusted.

## What it is designed to guard against

- **Misuse of commands:** role-based access on every interface, one controlling session per
  vehicle, and every command recorded before it is sent.
- **Tampering:** an append-only, hash-chained record of commands and changes; flight logs hashed
  when captured; signed exports.
- **Malformed input:** fuzz testing in CI, and imported files validated, never executed.
- **Supply chain:** pinned CI actions, a licence allow-list, a vulnerability scan on every pull
  request and automated dependency updates.
- **Data leaving the machine:** no phone-home, no telemetry; optional network features are off by
  default.

Contributors will find the full threat model in
[docs/06_Compliance_and_Quality/SECURITY.md](docs/06_Compliance_and_Quality/SECURITY.md).

## Known limits

- The local development setup runs without TLS and with default credentials. It is for one
  workstation, not a network.
- Multi-factor login is designed but optional; it becomes mandatory before command features ship.
- Much of the hardening above is designed, not built yet.
- MAVLink over radio links without MAVLink 2 signing can be spoofed. UAOP flags unsigned links but
  cannot fix the radio.
