#!/usr/bin/env bash
set -euo pipefail

mode="${1:-fix}"

files="$(
  find src include boards \
    -type f \
    \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) \
    | sort
)"

if [ -z "${files}" ]; then
  exit 0
fi

if [ "${mode}" = "--check" ]; then
  echo "${files}" | xargs clang-format --dry-run --Werror
else
  echo "${files}" | xargs clang-format -i
fi
