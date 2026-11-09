#!/usr/bin/env bash
set -euo pipefail
DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
# Compile once; never run this script through a multi-rank srun.
[[ $# -eq 1 ]] || { printf 'Usage: bash build.sh /absolute/run/f77pipeline.env\n' >&2; exit 2; }
exec bash "$DIR/runtime.sh" "$1" build
