#!/usr/bin/env bash
set -euo pipefail
DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=common.sh
source "$DIR/common.sh"
if [[ "${1:-}" == --check ]]; then
    shift
    [[ $# -eq 1 ]] || fq_die 'Usage: bash run.sh --check /absolute/run/f77pipeline.env'
    fq_load "$1"
    exec python3 "$DIR/../fqctl.py" check --project "$F77_PROJECT_JSON"
fi
[[ $# -eq 1 ]] || fq_die 'Usage: bash run.sh /absolute/run/f77pipeline.env'
# The caller's Slurm allocation controls task count and node placement.
# Do not wrap run.sh in an additional srun.
fq_load "$1"
exec srun --mpi=pmi2 --export=ALL --kill-on-bad-exit=1 \
    "${F77_SRUN_ARGS[@]}" bash "$DIR/runtime.sh" "$F77_ENV_FILE" run
