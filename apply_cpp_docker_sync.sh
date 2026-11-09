#!/usr/bin/env bash
# Apply from the repository root AFTER extracting the replacement ZIP.
# Delete only obsolete tracked entry points. No README or user configs touched.
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$root"
if [[ ! -f cpp_docker/fqctl.py || ! -f cpp_Standard/include/RuntimeConfig.hpp ]]; then
    echo "Error: extract into the Fourier_Quad_Pipeline repository root first." >&2
    exit 2
fi
if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    in_git=true
else
    in_git=false
fi
old="cpp_docker/.env.example"
if [[ -e "$old" || -L "$old" ]]; then
    if [[ "$in_git" == true ]]; then
        if ! git ls-files --error-unmatch -- "$old" >/dev/null 2>&1; then
            echo "Preserving untracked legacy file: $old"
            continue_cleanup=false
        elif ! git diff --quiet -- "$old" || ! git diff --cached --quiet -- "$old"; then
            echo "Preserving locally modified legacy file: $old"
            continue_cleanup=false
        else
            continue_cleanup=true
        fi
    else
        continue_cleanup=true
    fi
    if [[ "$continue_cleanup" == true ]]; then rm -f -- "$old"; fi
fi
old="cpp_docker/compose.yaml"
if [[ -e "$old" || -L "$old" ]]; then
    if [[ "$in_git" == true ]]; then
        if ! git ls-files --error-unmatch -- "$old" >/dev/null 2>&1; then
            echo "Preserving untracked legacy file: $old"
            continue_cleanup=false
        elif ! git diff --quiet -- "$old" || ! git diff --cached --quiet -- "$old"; then
            echo "Preserving locally modified legacy file: $old"
            continue_cleanup=false
        else
            continue_cleanup=true
        fi
    else
        continue_cleanup=true
    fi
    if [[ "$continue_cleanup" == true ]]; then rm -f -- "$old"; fi
fi
old="cpp_docker/compose.optional.yaml"
if [[ -e "$old" || -L "$old" ]]; then
    if [[ "$in_git" == true ]]; then
        if ! git ls-files --error-unmatch -- "$old" >/dev/null 2>&1; then
            echo "Preserving untracked legacy file: $old"
            continue_cleanup=false
        elif ! git diff --quiet -- "$old" || ! git diff --cached --quiet -- "$old"; then
            echo "Preserving locally modified legacy file: $old"
            continue_cleanup=false
        else
            continue_cleanup=true
        fi
    else
        continue_cleanup=true
    fi
    if [[ "$continue_cleanup" == true ]]; then rm -f -- "$old"; fi
fi
old="cpp_docker/runner/build-sif.slurm"
if [[ -e "$old" || -L "$old" ]]; then
    if [[ "$in_git" == true ]]; then
        if ! git ls-files --error-unmatch -- "$old" >/dev/null 2>&1; then
            echo "Preserving untracked legacy file: $old"
            continue_cleanup=false
        elif ! git diff --quiet -- "$old" || ! git diff --cached --quiet -- "$old"; then
            echo "Preserving locally modified legacy file: $old"
            continue_cleanup=false
        else
            continue_cleanup=true
        fi
    else
        continue_cleanup=true
    fi
    if [[ "$continue_cleanup" == true ]]; then rm -f -- "$old"; fi
fi
old="cpp_docker/runner/compile-pipeline.slurm"
if [[ -e "$old" || -L "$old" ]]; then
    if [[ "$in_git" == true ]]; then
        if ! git ls-files --error-unmatch -- "$old" >/dev/null 2>&1; then
            echo "Preserving untracked legacy file: $old"
            continue_cleanup=false
        elif ! git diff --quiet -- "$old" || ! git diff --cached --quiet -- "$old"; then
            echo "Preserving locally modified legacy file: $old"
            continue_cleanup=false
        else
            continue_cleanup=true
        fi
    else
        continue_cleanup=true
    fi
    if [[ "$continue_cleanup" == true ]]; then rm -f -- "$old"; fi
fi
old="cpp_docker/runner/cpppipeline.env.example"
if [[ -e "$old" || -L "$old" ]]; then
    if [[ "$in_git" == true ]]; then
        if ! git ls-files --error-unmatch -- "$old" >/dev/null 2>&1; then
            echo "Preserving untracked legacy file: $old"
            continue_cleanup=false
        elif ! git diff --quiet -- "$old" || ! git diff --cached --quiet -- "$old"; then
            echo "Preserving locally modified legacy file: $old"
            continue_cleanup=false
        else
            continue_cleanup=true
        fi
    else
        continue_cleanup=true
    fi
    if [[ "$continue_cleanup" == true ]]; then rm -f -- "$old"; fi
fi
old="cpp_docker/runner/cpppipeline.slurm"
if [[ -e "$old" || -L "$old" ]]; then
    if [[ "$in_git" == true ]]; then
        if ! git ls-files --error-unmatch -- "$old" >/dev/null 2>&1; then
            echo "Preserving untracked legacy file: $old"
            continue_cleanup=false
        elif ! git diff --quiet -- "$old" || ! git diff --cached --quiet -- "$old"; then
            echo "Preserving locally modified legacy file: $old"
            continue_cleanup=false
        else
            continue_cleanup=true
        fi
    else
        continue_cleanup=true
    fi
    if [[ "$continue_cleanup" == true ]]; then rm -f -- "$old"; fi
fi
old="cpp_docker/runner/inspect-cluster-mpi.sh"
if [[ -e "$old" || -L "$old" ]]; then
    if [[ "$in_git" == true ]]; then
        if ! git ls-files --error-unmatch -- "$old" >/dev/null 2>&1; then
            echo "Preserving untracked legacy file: $old"
            continue_cleanup=false
        elif ! git diff --quiet -- "$old" || ! git diff --cached --quiet -- "$old"; then
            echo "Preserving locally modified legacy file: $old"
            continue_cleanup=false
        else
            continue_cleanup=true
        fi
    else
        continue_cleanup=true
    fi
    if [[ "$continue_cleanup" == true ]]; then rm -f -- "$old"; fi
fi
old="cpp_docker/runner/mpi-smoke-test.slurm"
if [[ -e "$old" || -L "$old" ]]; then
    if [[ "$in_git" == true ]]; then
        if ! git ls-files --error-unmatch -- "$old" >/dev/null 2>&1; then
            echo "Preserving untracked legacy file: $old"
            continue_cleanup=false
        elif ! git diff --quiet -- "$old" || ! git diff --cached --quiet -- "$old"; then
            echo "Preserving locally modified legacy file: $old"
            continue_cleanup=false
        else
            continue_cleanup=true
        fi
    else
        continue_cleanup=true
    fi
    if [[ "$continue_cleanup" == true ]]; then rm -f -- "$old"; fi
fi
old="cpp_docker/runner/pull-sif.sh"
if [[ -e "$old" || -L "$old" ]]; then
    if [[ "$in_git" == true ]]; then
        if ! git ls-files --error-unmatch -- "$old" >/dev/null 2>&1; then
            echo "Preserving untracked legacy file: $old"
            continue_cleanup=false
        elif ! git diff --quiet -- "$old" || ! git diff --cached --quiet -- "$old"; then
            echo "Preserving locally modified legacy file: $old"
            continue_cleanup=false
        else
            continue_cleanup=true
        fi
    else
        continue_cleanup=true
    fi
    if [[ "$continue_cleanup" == true ]]; then rm -f -- "$old"; fi
fi
old="cpp_docker/runner/run-apptainer.sh"
if [[ -e "$old" || -L "$old" ]]; then
    if [[ "$in_git" == true ]]; then
        if ! git ls-files --error-unmatch -- "$old" >/dev/null 2>&1; then
            echo "Preserving untracked legacy file: $old"
            continue_cleanup=false
        elif ! git diff --quiet -- "$old" || ! git diff --cached --quiet -- "$old"; then
            echo "Preserving locally modified legacy file: $old"
            continue_cleanup=false
        else
            continue_cleanup=true
        fi
    else
        continue_cleanup=true
    fi
    if [[ "$continue_cleanup" == true ]]; then rm -f -- "$old"; fi
fi
old="cpp_docker/runner/site-env.example.sh"
if [[ -e "$old" || -L "$old" ]]; then
    if [[ "$in_git" == true ]]; then
        if ! git ls-files --error-unmatch -- "$old" >/dev/null 2>&1; then
            echo "Preserving untracked legacy file: $old"
            continue_cleanup=false
        elif ! git diff --quiet -- "$old" || ! git diff --cached --quiet -- "$old"; then
            echo "Preserving locally modified legacy file: $old"
            continue_cleanup=false
        else
            continue_cleanup=true
        fi
    else
        continue_cleanup=true
    fi
    if [[ "$continue_cleanup" == true ]]; then rm -f -- "$old"; fi
fi
old="cpp_docker/runner/tests/mpi_identity.cpp"
if [[ -e "$old" || -L "$old" ]]; then
    if [[ "$in_git" == true ]]; then
        if ! git ls-files --error-unmatch -- "$old" >/dev/null 2>&1; then
            echo "Preserving untracked legacy file: $old"
            continue_cleanup=false
        elif ! git diff --quiet -- "$old" || ! git diff --cached --quiet -- "$old"; then
            echo "Preserving locally modified legacy file: $old"
            continue_cleanup=false
        else
            continue_cleanup=true
        fi
    else
        continue_cleanup=true
    fi
    if [[ "$continue_cleanup" == true ]]; then rm -f -- "$old"; fi
fi
old="cpp_docker/runner/tests/science_stack_smoke.cpp"
if [[ -e "$old" || -L "$old" ]]; then
    if [[ "$in_git" == true ]]; then
        if ! git ls-files --error-unmatch -- "$old" >/dev/null 2>&1; then
            echo "Preserving untracked legacy file: $old"
            continue_cleanup=false
        elif ! git diff --quiet -- "$old" || ! git diff --cached --quiet -- "$old"; then
            echo "Preserving locally modified legacy file: $old"
            continue_cleanup=false
        else
            continue_cleanup=true
        fi
    else
        continue_cleanup=true
    fi
    if [[ "$continue_cleanup" == true ]]; then rm -f -- "$old"; fi
fi
old="cpp_docker/scripts/verify-image.sh"
if [[ -e "$old" || -L "$old" ]]; then
    if [[ "$in_git" == true ]]; then
        if ! git ls-files --error-unmatch -- "$old" >/dev/null 2>&1; then
            echo "Preserving untracked legacy file: $old"
            continue_cleanup=false
        elif ! git diff --quiet -- "$old" || ! git diff --cached --quiet -- "$old"; then
            echo "Preserving locally modified legacy file: $old"
            continue_cleanup=false
        else
            continue_cleanup=true
        fi
    else
        continue_cleanup=true
    fi
    if [[ "$continue_cleanup" == true ]]; then rm -f -- "$old"; fi
fi
echo "Obsolete tracked Docker/HPC entry points reviewed and cleaned."
echo "Generated project.json, fq-run and local data were not touched."
