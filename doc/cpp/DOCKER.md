# 🐳 C++ Docker Workstation Guide

[Home](../../README.md) · [C++ User Guide](CPP_GUIDE.md) · [Configuration](CONFIGURATION.md) · [Container Paths](PATHS.md) · [HPC](HPC.md)

Docker provides the `linux/amd64` C++17 scientific toolchain used by `cpp_Standard/` without requiring local CFITSIO, FFTW, Eigen, or MPI builds. The Python 3.8+ standard-library launcher `cpp_docker/fqctl.py` manages a **JSON project**, emits a C++ **INI**, and runs the existing `Fourier_Quad_Pipe` binary. Docker Compose is not required.

## 1. Initialize and Edit `project.json`

From the repository root:

```bash
python3 cpp_docker/fqctl.py init --json cpp_docker/project.json
```

Edit the generated `cpp_docker/project.json`. The launcher refuses to overwrite an existing JSON. Start with `cpp_docker/project.example.json` or `project.full.example.json` for the available fields. Required data-related entries are host `mounts` (which map storage into containers) and the `paths` values written to the INI.

Example excerpt (replace all sample host paths):

```json
{
  "version": 1,
  "source": "../cpp_Standard",
  "image": "ghcr.io/syoong-s/fourier_quad_pipeline/cpppipeline:latest",
  "ranks": 4,
  "build_jobs": 4,
  "run_directory": "./fq-run",
  "mounts": [
    {"host": "/survey/science", "container": "/data/science", "mode": "ro"},
    {"host": "/survey/dq", "container": "/data/dq", "mode": "ro"},
    {"host": "./results", "container": "/data/output", "mode": "rw", "create": true}
  ],
  "paths": {
    "init.science_root": "/data/science",
    "init.dq_root": "/data/dq",
    "init.output_root": "/data/output"
  },
  "environment": {"OMP_NUM_THREADS": "1"},
  "hpc": {"sif": "./cpppipeline.sif", "runtime": "apptainer", "srun_args": [], "environment": {}}
}
```

The image above is published by this repository's C++ image workflow. **Set `image` explicitly in your project JSON**: the value in the provided launcher template may differ, and an implicit default should not be relied upon. The chosen image must contain compatible libraries and MPI tooling.

## 2. Generate Run Configuration (`configure`)

```bash
python3 cpp_docker/fqctl.py configure --json cpp_docker/project.json
```

This generates the runtime directory (by default `cpp_docker/fq-run`) with a managed `pipeline.ini` and HPC environment file. The JSON `paths` map inserts container-visible strings into the INI. The launcher deliberately leaves most science keys commented to **inherit the C++ compiled defaults**. To change phase switches or scientific overrides, edit the **unmanaged** INI section and preserve the generated/managed boundaries. Re-running `configure` preserves user science edits where supported; it will refuse unrecognized marker layouts.

**Do not put native host paths inside the generated INI** unless the container also sees those exact paths. For complete mapping and read/write rules, use [Container Path Reference](PATHS.md).

## 3. Validate, Pull Image & Compile

```bash
python3 cpp_docker/fqctl.py check --json cpp_docker/project.json
python3 cpp_docker/fqctl.py pull --json cpp_docker/project.json
python3 cpp_docker/fqctl.py build --json cpp_docker/project.json
```

`check` verifies project structure, source signature, mounts, generated files, and configuration consistency without launching containers. `pull` obtains the image from the configured registry. `build` mounts the C++ source writable and rebuilds it with `make` inside the scientific image. Source must reside outside the generated runtime directory.

## 4. Run the Pipeline

```bash
python3 cpp_docker/fqctl.py run --json cpp_docker/project.json
```

The launcher supplies **only** `--config /fq-config/pipeline.ini` as application arguments; the `ranks` JSON value controls MPI ranks. Do not add old phase or dataset CLI flags. Run uses read-only mounts for the source, read-only or writable data mounts as declared, and ensures the compiled executable exists.

## Troubleshooting

| Problem | Action |
| :--- | :--- |
| Missing/invalid JSON | Run `init`, then edit `project.json`. |
| Unmapped path | Every non-null path must be covered by an appropriate host-to-container mount. |
| Source signature rejected | Confirm the source is the supplied `cpp_Standard/` with `RuntimeConfig.hpp` and `Makefile`. |
| Unknown INI setting | Compare against the supported [C++ parameter reference](CPP_PIPELINE_PARAMETERS.md). |
| `Executable missing` | Run `build` before `run`. |
| Docker not installed | Use [native build](CPP_GUIDE.md#build-and-run) or [Apptainer HPC](HPC.md). |
| Change not picked up | Re-run `configure` after JSON edits; rebuild after header or C++ source changes. |

The container is a toolchain/runtime image, not a bundle of scientific survey data. The host supplies inputs and persistence through mounts.
