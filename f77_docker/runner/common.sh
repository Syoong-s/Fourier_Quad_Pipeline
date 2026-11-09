#!/usr/bin/env bash
# Shared trusted configuration and container execution helpers.
set -euo pipefail

fq_die() { printf 'Error: %s\n' "$*" >&2; exit 2; }

fq_load() {
    local file="$1"
    [[ -r "$file" ]] || fq_die "generated env is not readable: $file"
    F77_ENV_FILE="$(cd -- "$(dirname -- "$file")" && pwd)/$(basename -- "$file")"
    # This file contains Bash assignments generated from trusted project JSON.
    # shellcheck disable=SC1090
    source "$F77_ENV_FILE"
}

fq_runtime() {
    F77_CONTAINER_RUNTIME="${F77_RUNTIME:-apptainer}"
    command -v "$F77_CONTAINER_RUNTIME" >/dev/null 2>&1 ||
        fq_die "container runtime not found: $F77_CONTAINER_RUNTIME"
    [[ -r "$F77_SIF" ]] || fq_die "SIF not readable: $F77_SIF"
}

fq_exec() {
    local action="$1"; shift
    local i key name prefix=APPTAINERENV_
    [[ "$(basename -- "$F77_CONTAINER_RUNTIME")" != *singularity* ]] || prefix=SINGULARITYENV_
    # Do not import user-defined hidden binds or container environment overrides.
    while IFS= read -r name; do
        case "$name" in
            APPTAINERENV_*|SINGULARITYENV_*) unset "$name" ;;
        esac
    done < <(compgen -e)
    unset APPTAINER_BIND APPTAINER_BINDPATH APPTAINER_MOUNT
    unset SINGULARITY_BIND SINGULARITY_BINDPATH SINGULARITY_MOUNT
    if [[ "$action" == run ]]; then
        # This code runs inside each srun task, so PMI_RANK is rank-specific.
        while IFS= read -r name; do
            case "$name" in
                PMI_*|PMI2_*|SLURM_*) export "${prefix}${name}=${!name}" ;;
            esac
        done < <(compgen -e)
    fi
    for key in "${F77_CONTAINER_ENV[@]}"; do export "${prefix}${key}"; done
    local bindmode=ro
    [[ "$action" != build ]] || bindmode=rw
    local options=(exec --cleanenv --no-eval --no-home --no-mount cwd,hostfs
                   --pwd "$F77_SOURCE_CONTAINER"
                   --bind "${F77_STAGED_SOURCE_HOST}:${F77_SOURCE_CONTAINER}:${bindmode}")
    if [[ "$action" == run ]]; then
        for i in "${!F77_MOUNT_HOSTS[@]}"; do
            options+=(--bind "${F77_MOUNT_HOSTS[i]}:${F77_MOUNT_TARGETS[i]}:${F77_MOUNT_MODES[i]}")
        done
    fi
    exec "$F77_CONTAINER_RUNTIME" "${options[@]}" "$F77_SIF" "$@"
}
