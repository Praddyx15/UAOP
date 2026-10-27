"""Sidecar config from the environment, same naming as services/common's ConfigStore
(UAOP_FOO__BAR overrides key foo.bar). Missing required values fail at startup."""

import json
import os
from dataclasses import dataclass, field
from pathlib import Path
from urllib.parse import urlsplit


class ConfigError(ValueError):
    pass


@dataclass(frozen=True)
class Config:
    data_dir: Path
    log_dir: Path
    flightmd_url: str = ""  # empty: the tune-fly-verify hand-back is off (ADR-0018 F29)
    flightmd_tokens: dict[str, str] = field(default_factory=dict, repr=False)
    flightmd_timeout_s: float = 10.0
    nats_url: str = ""


def _dir(env: dict[str, str], key: str) -> Path:
    value = env.get(key, "").strip()
    if not value:
        raise ConfigError(f"{key} is not set")
    path = Path(value).resolve()
    if not path.is_dir():
        raise ConfigError(f"{key}: {path} is not a directory")
    return path


def _tokens(path: str) -> dict[str, str]:
    # {"<airframe label>": "<X-Airframe-Token>"}; a file, so tokens stay out of the env dump
    try:
        raw = json.loads(Path(path).read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ConfigError(f"UAOP_FLIGHTMD__TOKENS_FILE: cannot read {path}: {exc}") from None
    if not isinstance(raw, dict) or not all(isinstance(k, str) and isinstance(v, str) and v for k, v in raw.items()):
        raise ConfigError("UAOP_FLIGHTMD__TOKENS_FILE must hold a JSON object of label -> token strings")
    return raw


def load(env: dict[str, str] | None = None) -> Config:
    env = dict(os.environ if env is None else env)
    url = env.get("UAOP_FLIGHTMD__URL", "").strip().rstrip("/")
    if url:
        parts = urlsplit(url)
        if parts.scheme not in ("http", "https") or not parts.hostname or parts.username or parts.password:
            raise ConfigError("UAOP_FLIGHTMD__URL must be an http(s) URL without credentials")
    tokens_file = env.get("UAOP_FLIGHTMD__TOKENS_FILE", "").strip()
    try:
        timeout = float(env.get("UAOP_FLIGHTMD__TIMEOUT_S", "10"))
    except ValueError:
        raise ConfigError("UAOP_FLIGHTMD__TIMEOUT_S must be a number") from None
    if not 0 < timeout <= 120:
        raise ConfigError("UAOP_FLIGHTMD__TIMEOUT_S must be in (0, 120]")
    return Config(
        data_dir=_dir(env, "UAOP_FLIGHT_LOG__DATA_DIR"),
        log_dir=_dir(env, "UAOP_FLIGHT_LOG__LOG_DIR"),
        flightmd_url=url,
        flightmd_tokens=_tokens(tokens_file) if tokens_file else {},
        flightmd_timeout_s=timeout,
        nats_url=env.get("UAOP_NATS_URL", "").strip(),
    )
