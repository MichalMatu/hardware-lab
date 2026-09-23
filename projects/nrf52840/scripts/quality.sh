#!/usr/bin/env bash
set -euo pipefail

scripts/format.sh --check
pio run
pio check --fail-on-defect medium
