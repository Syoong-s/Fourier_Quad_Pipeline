#!/usr/bin/env bash
# Validate the current Fortran Docker and JSON-launcher assets before publishing.
set -euo pipefail

PROJECT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
REPOSITORY_DIR="$(cd -- "${PROJECT_DIR}/.." && pwd)"

require_file() {
    [[ -s "$1" ]] || {
        printf 'Missing or empty required file: %s\n' "$1" >&2
        exit 1
    }
}

check_layout() {
    local file
    for file in \
        "$PROJECT_DIR/Dockerfile" \
        "$PROJECT_DIR/fqctl.py" \
        "$PROJECT_DIR/project.example.json" \
        "$PROJECT_DIR/config/f77stack.conf" \
        "$PROJECT_DIR/config/lapack-make.inc" \
        "$PROJECT_DIR/patches/gcc-4.8.5-el8-compat.patch" \
        "$PROJECT_DIR/runner/common.sh" \
        "$PROJECT_DIR/runner/build.sh" \
        "$PROJECT_DIR/runner/run.sh" \
        "$PROJECT_DIR/runner/runtime.sh" \
        "$PROJECT_DIR/runner/pull-sif.sh" \
        "$PROJECT_DIR/runner/tests/mpi_identity.f" \
        "$PROJECT_DIR/runner/tests/mpi_lapack_smoke.f" \
        "$PROJECT_DIR/scripts/verify-image.sh" \
        "$PROJECT_DIR/tests/test_fqctl.py" \
        "$REPOSITORY_DIR/f77/Makefile" \
        "$REPOSITORY_DIR/f77/para.inc" \
        "$REPOSITORY_DIR/f77/00_psf_module.f"; do
        require_file "$file"
    done
}

check_public_payload() {
    if find "$PROJECT_DIR" -type f \
        \( -name '*.tar' -o -name '*.tar.gz' -o -name '*.tar.bz2' \
           -o -name '*.tar.xz' -o -name '*.sif' \) \
        -print -quit | grep -q .; then
        printf 'Archive or SIF present in the public Fortran Docker project.\n' >&2
        return 1
    fi
    if find "$PROJECT_DIR" -type f -size +95M -print -quit | grep -q .; then
        printf 'Fortran Docker project contains a file larger than 95 MiB.\n' >&2
        return 1
    fi
    if grep -n -E '^[[:space:]]*COPY[[:space:]]+(sources/|\.\./)' \
        "$PROJECT_DIR/Dockerfile"; then
        printf 'Dockerfile must not bundle local pipeline source archives.\n' >&2
        return 1
    fi
}

check_scripts() {
    local script
    for script in "$PROJECT_DIR"/runner/*.sh "$PROJECT_DIR"/scripts/*.sh; do
        [[ -f "$script" ]] || continue
        bash -n "$script"
    done
    if grep -R -I -l $'\r$' \
        --exclude-dir=__pycache__ --exclude='*.pyc' "$PROJECT_DIR" | grep -q .; then
        printf 'CRLF line endings found in Fortran Docker project.\n' >&2
        return 1
    fi
    if ! awk 'length($0) > 72 { exit 1 }' \
        "$PROJECT_DIR/runner/tests/mpi_identity.f" \
        "$PROJECT_DIR/runner/tests/mpi_lapack_smoke.f"; then
        printf 'Fixed-form Fortran smoke test exceeds 72 columns.\n' >&2
        return 1
    fi
}

check_launcher() {
    PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover \
        -s "$PROJECT_DIR/tests" -p 'test_*.py' -v
}

main() {
    check_layout
    check_public_payload
    check_scripts
    check_launcher
    printf 'Fortran Docker publication checks passed.\n'
}

main "$@"
