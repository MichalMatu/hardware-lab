from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any, Mapping

try:
    import tomllib
except ModuleNotFoundError:  # KiCad 10 on macOS currently embeds Python 3.9.
    tomllib = None

from library.host_profile import HostIdentity


SUPPORTED_STATUSES = frozenset({"concept", "prototype", "production"})


@dataclass(frozen=True)
class BoardManifest:
    id: str
    revision: str
    status: str
    host_profile: str
    modules: tuple[str, ...]

    @property
    def is_production(self) -> bool:
        return self.status == "production"


def _require_table(data: Mapping[str, Any], key: str) -> Mapping[str, Any]:
    value = data.get(key)
    if not isinstance(value, Mapping):
        raise ValueError(f"board.toml must contain a [{key}] table")
    return value


def validate_board_manifest(
    data: Mapping[str, Any], *, host_identity: HostIdentity | None = None
) -> BoardManifest:
    board = _require_table(data, "board")
    host = _require_table(data, "host")
    features = _require_table(data, "features")

    board_id = str(board.get("id", "")).strip()
    revision = str(board.get("revision", "")).strip()
    status = str(board.get("status", "")).strip()
    host_profile = str(host.get("profile", "")).strip()
    raw_modules = features.get("modules", [])

    if not board_id:
        raise ValueError("board.id is required")
    if not revision:
        raise ValueError("board.revision is required")
    if status not in SUPPORTED_STATUSES:
        raise ValueError(
            f"Unsupported board.status '{status}'. Expected one of: "
            f"{', '.join(sorted(SUPPORTED_STATUSES))}"
        )
    if not host_profile:
        raise ValueError("host.profile is required")
    if not isinstance(raw_modules, list):
        raise ValueError("features.modules must be a TOML array")

    modules: list[str] = []
    seen: set[str] = set()
    for raw_module in raw_modules:
        if not isinstance(raw_module, str):
            raise ValueError("features.modules accepts only strings")
        module = raw_module.strip()
        if not module:
            raise ValueError("features.modules cannot contain empty names")
        if module in seen:
            raise ValueError(f"Duplicate feature module: {module}")
        if "/" in module or "\\" in module or module in {".", ".."}:
            raise ValueError(f"Invalid feature module name: {module}")
        seen.add(module)
        modules.append(module)

    manifest = BoardManifest(
        id=board_id,
        revision=revision,
        status=status,
        host_profile=host_profile,
        modules=tuple(modules),
    )

    if host_identity is not None:
        if host_identity.id != manifest.host_profile:
            raise ValueError(
                f"Board host '{manifest.host_profile}' does not match loaded host "
                f"'{host_identity.id}'"
            )
        if host_identity.status == "deprecated":
            raise ValueError(
                f"Board '{manifest.id}' cannot use deprecated host '{host_identity.id}'"
            )
        if manifest.is_production and not host_identity.is_verified:
            raise ValueError(
                f"Production board '{manifest.id}' requires a verified host; "
                f"'{host_identity.id}' is '{host_identity.status}'"
            )

    return manifest


def load_board_manifest(
    path: str | Path, *, host_identity: HostIdentity | None = None
) -> BoardManifest:
    if tomllib is None:
        raise RuntimeError(
            "TOML loading requires Python 3.11+ (tomllib) or a future explicit "
            "TOML dependency. Importing this module remains safe under KiCad Python 3.9."
        )

    manifest_path = Path(path)
    data = tomllib.loads(manifest_path.read_text(encoding="utf-8"))
    manifest = validate_board_manifest(data, host_identity=host_identity)

    if manifest_path.parent.name != manifest.id:
        raise ValueError(
            f"board.id '{manifest.id}' must match board directory "
            f"'{manifest_path.parent.name}'"
        )

    return manifest
