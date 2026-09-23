#!/usr/bin/env bash
set -euo pipefail

pre-commit install --hook-type pre-commit --hook-type pre-push
