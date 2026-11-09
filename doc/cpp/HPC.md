# 🏔️ C++ HPC Runner Guide — Slurm + Apptainer

[Home](../../README.md) · [C++ User Guide](CPP_GUIDE.md) · [Docker](DOCKER.md) · [Paths](PATHS.md)

Use the packaged C++ scientific toolchain on `linux/amd64` HPC nodes via Apptainer/Singularity and Slurm PMI2. A compatible site environment is required. The supplied runner does not assume an allocation policy; set partitions, nodes, rank count, CPUs, memory, and walltime in **your own Slurm job script**.

## 1. Prepare Configuration & SIF Image

On a login node with Python 3.8+, fill `cpp_docker/project.json` as described in [Docker](DOCKER.md), including the `hpc` object and host mounts. Then:

```bash
python3 cpp_docker/fqctl.py configure --json cpp_docker/project.json
python3 cpp_docker/fqctl.py check --json cpp_docker/project.json
```

The generated runtime directory contains `pipeline.ini` and `cpppipeline.env`. The SIF path is taken from `hpc.sif` in the JSON. Obtain the image using the source image reference:

```bash
# Supply the absolute generated environment path.
bash cpp_docker/runner/run.sh --check /absolute/path/to/cpppipeline.env
```

In this repository, the shipped `cpp_docker/runner/` contains `build.sh`, `common.sh`, `run.sh`, and `runtime.sh`. There is **no separate `pull-sif.sh`**. Pull the image explicitly using the Apptainer tool supplied by your cluster, for example:

```bash
apptainer pull /absolute/path/to/cpppipeline.sif docker://ghcr.io/syoong-s/fourier_quad_pipeline/cpppipeline:latest
```

Replace the example image with the `image` value in your JSON if different. The directory must be writable and the SIF visible to compute nodes.

## 2. Validate and Compile Once

```bash
bash cpp_docker/runner/build.sh /absolute/path/to/cpppipeline.env
```

Build is a **single-task** operation; do not launch `build.sh` under a multi-rank `srun`. It compiles `cpp_Standard` through the scientific container stack. If changing `config/*.hpp`, rebuild before launching.

## 3. Launch from Your Slurm Job Script

A site-specific example (replace resources and module commands):

```bash
#!/usr/bin/env bash
#SBATCH --job-name=fq-cpp
#SBATCH --nodes=2
#SBATCH --ntasks=32
#SBATCH --time=02:00:00
set -euo pipefail
# module load apptainer
bash cpp_docker/runner/run.sh /absolute/path/to/cpppipeline.env
```

Submit from the repository root with `sbatch your-job.slurm`. The runner uses the existing Slurm allocation and starts **one** `srun --mpi=pmi2` step; do not wrap the runner in another `srun`. It forwards rank-specific PMI/Slurm variables through the container runtime and ultimately invokes the C++ executable with `--config`.

## Runtime Execution Rules

- The image and site must support the same PMI2 launch method; do not inject host MPI libraries into the container.
- Every MPI rank must see the generated INI, executable, input mounts, and output destinations at consistent paths.
- Data mounts remain governed by the JSON `mounts` entries; host and container paths are not interchangeable.
- `hpc.srun_args` accepts only a documented limited set of binding/distribution options. Resource requests belong in the Slurm job script.
- `check` does not imply that the scientific executable has been compiled. Build before a full run.

See [Docker](DOCKER.md) for JSON setup and [Paths](PATHS.md) for mount examples.
