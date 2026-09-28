from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any, Mapping

try:
    import tomllib
except ModuleNotFoundError:  # KiCad 10 on macOS currently embeds Python 3.9.
    tomllib = None


SUPPORTED_FAMILIES = frozenset({"esp32-s3", "esp32-c6"})
SUPPORTED_STATUSES = frozenset({"draft", "verified", "deprecated"})


@dataclass(frozen=True)
class HostIdentity:
    id: str
    family: str
    status: str
    vendor: str
    model: str

    @property
    def is_verified(self) -> bool:
        return self.status == "verified"


def validate_host_identity(
    data: Mapping[str, Any], *, require_verified: bool = False
) -> HostIdentity:
    host = data.get("host")
    if not isinstance(host, Mapping):
        raise ValueError("host.toml must contain a [host] table")

    host_id = str(host.get("id", "")).strip()
    family = str(host.get("family", "")).strip()
    status = str(host.get("status", "")).strip()
    vendor = str(host.get("vendor", "")).strip()
    model = str(host.get("model", "")).strip()

    if not host_id:
        raise ValueError("host.id is required")
    if family not in SUPPORTED_FAMILIES:
        raise ValueError(
            f"Unsupported host.family '{family}'. Expected one of: "
            f"{', '.join(sorted(SUPPORTED_FAMILIES))}"
        )
    if status not in SUPPORTED_STATUSES:
        raise ValueError(
            f"Unsupported host.status '{status}'. Expected one of: "
            f"{', '.join(sorted(SUPPORTED_STATUSES))}"
        )

    identity = HostIdentity(
        id=host_id,
        family=family,
        status=status,
        vendor=vendor,
        model=model,
    )

    if require_verified:
        if not identity.is_verified:
            raise ValueError(
                f"Host '{identity.id}' is '{identity.status}', not verified; "
                "it cannot be used as a production PCB host"
            )
        if not identity.vendor or not identity.model:
            raise ValueError(
                f"Verified host '{identity.id}' must define host.vendor and host.model"
            )

    return identity


def load_host_identity(path: str | Path, *, require_verified: bool = False) -> HostIdentity:
    if tomllib is None:
        raise RuntimeError(
            "TOML loading requires Python 3.11+ (tomllib) or a future explicit "
            "TOML dependency. Importing this module remains safe under KiCad Python 3.9."
        )

    profile_path = Path(path)
    data = tomllib.loads(profile_path.read_text(encoding="utf-8"))
    identity = validate_host_identity(data, require_verified=require_verified)

    if profile_path.parent.name != identity.id:
        raise ValueError(
            f"host.id '{identity.id}' must match profile directory "
            f"'{profile_path.parent.name}'"
        )

    return identity
