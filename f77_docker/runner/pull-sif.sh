#!/usr/bin/env bash
set -euo pipefail
DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "$DIR/common.sh"
# New launcher: explicit generated env. Historical no-argument usage remains
# accepted so existing HPC scripts do not require documentation changes.
if [[ $# -eq 1 ]]; then
    fq_load "$1"
elif [[ $# -eq 0 ]]; then
    fq_load "${F77_RUNNER_ENV_FILE:-$DIR/f77pipeline.env}"
else
    fq_die 'Usage: bash pull-sif.sh [generated-f77pipeline.env]'
fi
image="${F77_OCI_IMAGE:-${OCI_IMAGE_URI:-}}"
[[ -n "$image" ]] || fq_die 'Set F77_OCI_IMAGE (generated) or OCI_IMAGE_URI (legacy)'
[[ -n "${F77_SIF:-}" && "$F77_SIF" == /* ]] || fq_die 'F77_SIF must be an absolute path'
[[ ! -e "$F77_SIF" ]] || fq_die "SIF already exists: $F77_SIF"
mkdir -p -- "$(dirname -- "$F77_SIF")"
runtime="${F77_RUNTIME:-${APPTAINER_BIN:-}}"
if [[ -z "$runtime" ]]; then
    if command -v apptainer >/dev/null 2>&1; then
        runtime=apptainer
    elif command -v singularity >/dev/null 2>&1; then
        runtime=singularity
    else
        fq_die 'apptainer or singularity not found'
    fi
fi
command -v "$runtime" >/dev/null 2>&1 || fq_die "runtime not found: $runtime"
exec "$runtime" pull "$F77_SIF" "docker://$image"
