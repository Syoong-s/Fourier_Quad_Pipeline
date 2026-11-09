#!/usr/bin/env bash
# Compatibility hook for .github/workflows/docker-images.yml in the legacy repo.
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
source_root="$(cd -- "$root/.." && pwd)/cpp_Standard"
for required in     "$root/Dockerfile" "$root/pixi.toml" "$root/pixi.lock"     "$root/fqctl.py" "$root/schema.py"     "$source_root/Makefile" "$source_root/include/RuntimeConfig.hpp"; do
    [[ -s "$required" ]] || { echo "Missing/empty required file: $required" >&2; exit 1; }
done
grep -F 'STACK_PREFIX ?=' "$source_root/Makefile" >/dev/null
grep -F 'FROM ${RUNTIME_IMAGE} AS runtime' "$root/Dockerfile" >/dev/null
python3 -m compileall -q "$root/fqctl.py" "$root/schema.py"
for script in "$root"/runner/*.sh; do bash -n "$script"; done
python3 -m unittest discover -s "$root/tests" -v
echo 'Neo-style C++ launcher publication checks passed.'
