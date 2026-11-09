# 🏔️ Fortran77 HPC Runner Guide — Slurm + Apptainer

[Home](../../README.md) · [F77 User Guide](F77_GUIDE.md) · [Docker](DOCKER.md) · [Container Paths](PATHS.md)

Run the F77 scientific toolchain on `linux/amd64` Slurm clusters with Apptainer/Singularity. The supplied runner expects **PMI2** and the container's **MPICH 4.1.2** runtime; do not replace it with host OpenMPI libraries. Confirm compatibility with your cluster's Slurm PMI implementation before scaling to multiple nodes.

## 1. Configure Paths and Acquire a SIF

Use the F77 project JSON described in [Docker](DOCKER.md) and set the `hpc` block:

```json
{
  "hpc": {
    "sif": "./f77pipeline.sif",
    "runtime": "apptainer",
    "mpi": "pmi2",
    "srun_args": [],
    "environment": {}
  }
}
```

This fragment belongs inside a complete project JSON. On a login node with Python 3.8+ and shared storage:

```bash
python3 f77_docker/fqctl.py configure --project f77_docker/project.json
python3 f77_docker/fqctl.py check --project f77_docker/project.json
```

`configure` publishes an isolated Fortran source snapshot and generates `f77pipeline.env` in the configured run directory. The generated environment includes the staged source path, input mounts, target SIF, image reference and launcher options. It should not be manually edited.

Obtain the SIF with Apptainer, using the project's image reference. If the image has already been selected in the generated environment:

```bash
bash f77_docker/runner/pull-sif.sh /absolute/path/to/f77pipeline.env
```

This script refuses to overwrite an existing SIF. The login node requires access to the container registry and a writable SIF destination.

## 2. Explicit Configuration Check

```bash
bash f77_docker/runner/run.sh --check /absolute/path/to/f77pipeline.env
```

The check resolves the project JSON and validates the generated source/configuration without starting an MPI science job. It does not guarantee that an executable was compiled.

## 3. Compile Once

```bash
bash f77_docker/runner/build.sh /absolute/path/to/f77pipeline.env
```

Build compiles a **single staged source tree once** within the toolchain image. Do not execute `build.sh` from a multi-rank `srun`; avoid concurrent writes to the same staged binary. If `.inc` source settings change, rerun `configure` and rebuild.

## 4. Launch from Your Own Slurm Job Script

Use a site-specific job script that requests resources and starts only **one** F77 runner invocation:

```bash
#!/usr/bin/env bash
#SBATCH --job-name=fq-f77
#SBATCH --nodes=2
#SBATCH --ntasks=32
#SBATCH --time=02:00:00
set -euo pipefail
# module load apptainer
bash f77_docker/runner/run.sh /absolute/path/to/f77pipeline.env
```

Submit with `sbatch job.slurm`. The runner performs its own `srun --mpi=pmi2` and forwards rank-specific environment values; do **not** wrap it inside another `srun`. On clusters that do not support the provided PMI2 path, use a compatible native MPI stack or adapt and validate the runner before production use.

## Notes on Configuration and Compatibility

- Both the staged executable and data paths must be visible on the compute nodes.
- The container's `para.inc` path constants are already staged; changing the JSON without reconfiguration creates a stale deployment.
- F77 still takes **one exposure-list path**; there is no INI argument.
- Slurm resource selection belongs in your batch script, not the JSON `hpc.srun_args` list. Supported `srun_args` are limited to binding/distribution controls.
- Treat generated `f77pipeline.env` as **trusted executable shell configuration** and do not share it with untrusted users.

See [Container Paths](PATHS.md) for mounting exposure lists, catalogs, and writable processing trees.
