#!/usr/bin/env bash
set -euo pipefail
DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=common.sh
source "$DIR/common.sh"
[[ $# -eq 2 ]] || fq_die 'Internal entry requires env path and action'
fq_load "$1"
action="$2"
[[ "$action" == build || "$action" == run ]] || fq_die "Invalid action: $action"
fq_runtime
if [[ "$action" == build ]]; then
    fq_exec build bash -c \
        'set -euo pipefail; make clean; exec make -j "$1" LAPACK_LIB_DIR=/opt/f77stack/lib CFITSIO_LIB_DIR=/opt/f77stack/lib CFITSIO_LIB=/opt/f77stack/lib/libcfitsio.so' \
        fq-build "$F77_BUILD_JOBS"
else
    fq_exec run "$F77_EXECUTABLE" "$F77_EXPO_LIST_CONTAINER"
fi
