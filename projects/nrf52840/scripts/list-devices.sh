#!/usr/bin/env sh
set -eu

pio device list
printf '\nVolumes:\n'
ls -1 /Volumes 2>/dev/null || true

