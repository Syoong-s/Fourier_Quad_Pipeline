# 🐳 Fortran77 Docker Workstation Guide

[Home](../../README.md) · [F77 User Guide](F77_GUIDE.md) · [Configuration](CONFIGURATION.md) · [Container Paths](PATHS.md) · [HPC](HPC.md)

Docker supplies the historical **GNU Fortran 4.8.5 + MPICH 4.1.2 + CFITSIO 4.3.1 + LAPACK 3.8.0** toolchain for `linux/amd64`. `f77_docker/fqctl.py` requires only the Python 3.8+ standard library. Docker Compose is not required.

**Important:** The JSON launcher prepares an **isolated Fortran source snapshot** and an HPC environment file. It does **not** create a C++-style INI, and the actual F77 executable still accepts a **required positional exposure-list pathname**.

## 1. Create a Deployment Project

From the repository root:

```bash
python3 f77_docker/fqctl.py init --project f77_docker/project.json
```

Edit the generated JSON. The launcher will not overwrite an existing project. Alternatively start from `f77_docker/project.example.json` and change the sample host paths. Typical fields include `source`, `run_directory`, `ranks`, `build_jobs`, `mounts`, `paths`, `environment`, and `hpc`.

A minimal illustrative excerpt (not a complete runnable project without existing inputs):

```json
{
  "version": 1,
  "source": "../f77",
  "run_directory": "./fq-run",
  "image": "ghcr.io/syoong-s/fourier_quad_pipeline/f77pipeline:gnu4.8.5",
  "ranks": 4,
  "build_jobs": 4,
  "mounts": [
    {"host": "/survey/processed", "container": "/data/processed", "mode": "rw"},
    {"host": "/survey/catalogs", "container": "/data/catalogs", "mode": "ro"}
  ],
  "paths": {
    "astrometry_cat": "/data/catalogs/gaia",
    "source_cat": "/data/catalogs/external",
    "flat_path": null,
    "psf_path": null,
    "exposure_list": "/data/processed/expo_gband.list"
  },
  "environment": {},
  "hpc": {"sif": "./f77pipeline.sif", "runtime": "apptainer", "mpi": "pmi2", "srun_args": [], "environment": {}}
}
```

Mount host paths must exist (unless a valid writable-directory mount uses `create: true`), and `paths.exposure_list` must refer to an **existing file** inside the container mapping. Relative host paths are resolved from the JSON file's directory. See [PATHS.md](PATHS.md) for the exact schema and limitations.

## 2. Generate a Separate Build Configuration

```bash
python3 f77_docker/fqctl.py configure --project f77_docker/project.json
```

`configure` stages a copy of the `f77/` scientific source in the run directory and applies **only** the JSON-selected Fortran path constants (`ASTROMETRY_CAT`, `SOURCE_CAT`, `FLAT_PATH`, `PSF_PATH`) to its staged `para.inc`. It also writes `f77pipeline.env` for the HPC runner. The original checked-in `f77/para.inc` is not modified.

Changes to **numeric scientific constants** must be made in the source `.inc` files before re-running `configure`. The launcher fingerprints the source and rejects stale snapshots.

## 3. Check, Pull Image and Build

```bash
python3 f77_docker/fqctl.py check --project f77_docker/project.json
python3 f77_docker/fqctl.py pull --project f77_docker/project.json
python3 f77_docker/fqctl.py build --project f77_docker/project.json
```

`check` validates JSON, source snapshot, file mounts and generated configuration. `pull` downloads the configured Docker image; `build` recompiles the staged source in a writable container source mount using its historical toolchain. The output binary resides in the staged source directory, not in the original `f77/` tree.

## 4. Run with One Command

```bash
python3 f77_docker/fqctl.py run --project f77_docker/project.json
```

The launcher runs the configured MPI rank count and supplies `paths.exposure_list` as the **first application argument** to the compiled F77 executable. No INI file or `--config` argument is involved.

## Troubleshooting

| Symptom | Check |
| :--- | :--- |
| `paths.exposure_list is required` | Supply a file path inside a declared container mount. |
| Staged source is stale | Re-run `configure` after changing source includes or JSON. |
| Build succeeds but run refuses | Check compiled executable in the run directory and execute `check`. |
| Missing chip images | Paths inside the top-level and per-exposure lists must be container-visible. |
| Wrong catalog tiles | Compiled `para.inc` paths in the staged source and actual mounted directories must agree. |
| Docker cannot run on the host | Use [native F77](F77_GUIDE.md#build-and-run) or [Slurm + Apptainer](HPC.md). |

No survey data, private cluster paths or scientific source trees should be baked into the published toolchain image.
