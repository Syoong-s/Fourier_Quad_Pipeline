# 🛠️ C++ Pipeline User Guide

[Home](../../README.md) · [Quick Start](CONFIGURATION.md) · [Data Preparation](DATA_PREPARATION.md) · [Parameter Reference](CPP_PIPELINE_PARAMETERS.md) · [Docker](DOCKER.md) · [HPC](HPC.md)

This guide covers native Linux/HPC deployment of the `Fourier_Quad_Pipe` executable under `cpp_Standard/`. The six top-level workflow phases, nine numerical stages, and optional runtime INI are features of **this C++ implementation only**.

---

## Environment and Dependencies

### Required toolchain and scientific libraries

| Dependency | Purpose | Reference container stack |
| :--- | :--- | :--- |
| C++17 compiler and `mpicxx` | Scientific kernel compilation and MPI communication | GCC 12.3.0, OpenMPI 4.1.8 |
| CFITSIO | Compressed FITS/FZ files and headers | 4.6.4 |
| FFTW3 and FFTW3f | Double/single precision Fourier transforms | 3.3.11 |
| Eigen3 | Linear algebra headers | Locked scientific stack |
| BLAS/LAPACK | Dense numerical solves | Locked scientific stack |
| `make`, Linux filesystem | Incremental build, data and output paths | x86_64 Linux |

The reference container stack is built by `cpp_docker/Dockerfile` using `pixi.toml` and `pixi.lock`; a native build may use other compatible versions. Never mix different MPI libraries at compilation and runtime. Some scientific libraries are supplied through the image rather than the host OS.

### Supported scientific capabilities (`cpp_Standard/`)

`process_astrocat` tiles raw Gaia catalogs; `process_extcat` tiles raw source catalogs; `process_init` extracts multi-extension Science/DQ archives; `process_main` performs scientific measurement; `process_rearr` redistributes catalogs spatially; `process_fd` runs a field-distortion shear analysis. Each phase can be selected independently if its input artifacts already exist.

The default camera constants describe a DECam-like instrument. Check image geometry, `CCDNUM`, catalog columns, and input paths before processing a new survey. Scientific branch compatibility must be validated with representative data.

## Configuration

### Setting up `pipeline.ini`

From the repository root:

```bash
cd cpp_Standard
cp pipeline.example.ini pipeline.ini
```

The example intentionally leaves most keys commented out. Replace site-specific input/output paths and inspect phase switches:

```ini
[init]
science_root = /data/science
dq_root = /data/dq
output_root = /data/processed
datasets = gband:c4d_
contains =

[lensing]
astrometry_cat = /data/catalogs/gaia
source_cat = /data/catalogs/external

[process]
run_process_init = true
run_process_main = true
run_process_rearr = false
run_process_fd = false
```

The leading whitespace before an INI key is allowed. The `contains =` above deliberately clears the original compiled filename token filter (`v1`). Do not copy these paths unmodified. Details: [Configuration Quick Start](CONFIGURATION.md) and [all INI keys](CPP_PIPELINE_PARAMETERS.md).

### INI syntax rules: omitted vs. empty

| Configuration | Effect |
| :--- | :--- |
| `# pixel_size =` or no `pixel_size` key | Retain the value from `config/LensingConfig.hpp` (`0.2628` arcsec/pixel). |
| `pixel_size = 0.30` | Apply the new value to every MPI rank. |
| `pixel_size =` | **Error**: an empty number does not mean “inherit”. |
| `contains =` | Intentionally clears the list of archive-basename filters. |
| `source_cat = /data/catalogs/external` | Sets the effective canonical source-tile directory. |

The application supports **only** `--config PATH` and `--config=PATH`; without these, it uses its compiled header defaults and does not search for an INI file. Old `--help`, `--dataset`, `--run-main` and positional exposure-list options are not supported by the executable. The `main.cpp` MPI startup reads the INI on rank 0, broadcasts its text, validates options on each rank, and then executes the selected phases.

Changing an INI value requires no rebuild. Changing any `config/*.hpp` compile-time constant requires a clean rebuild. The parser rejects unknown INI sections/keys; there are no independent `mag_g_column` through `mag_y_column` options in this C++ program.

## Build and Run

### 1. Compile the executable

```bash
cd cpp_Standard
make -j4
```

On a custom scientific stack:

```bash
make clean
make -j4 STACK_PREFIX=/opt/cppstack
```

Or provide the Eigen include root separately:

```bash
make -j4 EIGEN_INCLUDE=/opt/eigen/include/eigen3
```

The project Makefile uses `mpicxx`, `-std=c++17` and links `-lcfitsio -lfftw3 -lfftw3f -llapack -lblas -lm`. `STACK_PREFIX` also supplies compiler includes and an ELF library runtime search path. The source-level configuration-only test is:

```bash
make config-test
```

### 2. Launch with MPI

```bash
# From cpp_Standard/
mpirun -np 4 ./Fourier_Quad_Pipe --config pipeline.ini

# Without --config, only compiled defaults are used:
mpirun -np 4 ./Fourier_Quad_Pipe
```

Every MPI rank must have access to the same input/output tree. Use a site's approved MPI launcher and resource allocation. The configured `output_root` needs write permission and enough storage for chip products and catalogs.

## Workflow Phases & Main Stages

### Top-level phase execution order

| Order | INI `[process]` selector | Operation | Compiled default |
| :---: | :--- | :--- | :---: |
| 1 | `run_process_astrocat` | Build one-degree Gaia tile catalogs | `false` |
| 2 | `run_process_extcat` | Build canonical external source tiles | `false` |
| 3 | `run_process_init` | Extract Science/DQ and write exposure lists | `true` |
| 4 | `run_process_main` | Run the numerical FQ pipeline | `true` |
| 5 | `run_process_rearr` | Spatially partition output catalogs | `true` |
| 6 | `run_process_fd` | Field-distortion shear test | `false` |

**Catalog producers and consumers have separate path settings.** For Gaia, `[astrocat].output_directory` does not automatically set `[lensing].astrometry_cat`. The source-catalog producer output and the main consumer `source_cat` share an effective directory mapping.

### Nine-stage scientific pipeline (`process_main`)

The `[lensing].process_stage` integer is a product of prime factors. Each factor enables one main-processing stage:

| Prime | Stage | Operation |
| :---: | :---: | :--- |
| 2 | 1 | Image preprocessing, masks, noise/background |
| 3 | 2 | Gaia astrometry |
| 5 | 3 | Source detection, deblending, stamps |
| 7 | 4 | Stage-1 Fourier transforms |
| 11 | 5 | PSF star selection/model construction |
| 13 | 6 | Stage-2 Fourier preparation |
| 17 | 7 | Fourier_Quad shear measurement |
| 19 | 8 | Exposure statistics/diagnostics |
| 23 | 9 | Combined catalog products |

Only exclude a stage if all required intermediate outputs already exist. Stage 9 requires Stage 8 and the INI validator enforces their combination.

## Common Run Recipes

**Initialize and measure a small dataset using prepared reference tiles:**

```ini
[process]
run_process_astrocat = false
run_process_extcat = false
run_process_init = true
run_process_main = true
run_process_rearr = false
run_process_fd = false
```

**Reprocess existing initialized exposures without re-extracting archives:**

```ini
[process]
run_process_init = false
run_process_main = true
run_process_rearr = false
run_process_fd = false
expo_list = /data/processed/expo_gband.list
```

**Prepare raw catalog tiles without running the numerical phases:**

```ini
[process]
run_process_astrocat = true
run_process_extcat = true
run_process_init = false
run_process_main = false
run_process_rearr = false
run_process_fd = false

[astrocat]
input_directory = /data/catalogs/gaia_raw
output_directory = /data/catalogs/gaia_tiles

[extcat]
input_directory = /data/catalogs/source_raw
output_directory = /data/catalogs/source_tiles
```

## Output Directory Structure & Products

`[init].output_root` is the parent of the prepared dataset tree, for example:

```text
/data/processed/
├── gband/
│   ├── science/<exposure>/<chip>.fits
│   ├── dqmask/<exposure>/<chip>.fits
│   ├── expolists/<exposure>.list
│   ├── stamps/...
│   ├── astrometry/...
│   └── result/...
├── expo_gband.list
└── fits_gband.list
```

Additional main-stage products use fixed relative names defined in the source configuration headers and output layout. Rearrangement and FD write separate products controlled by their respective output-path settings. For file naming and required inputs, see [Data Preparation](DATA_PREPARATION.md) and [Path Reference](PATHS.md).

## Troubleshooting

| Symptom | Check |
| :--- | :--- |
| `unknown option` / `--help` rejected | Use only `--config pipeline.ini` or compiled defaults. |
| `cannot open INI file` | Give an explicit path visible to MPI rank 0. |
| `unknown key` | Check the exact `[section].key` list in the [parameter reference](CPP_PIPELINE_PARAMETERS.md). |
| No exposures selected | Review `datasets`, `contains` (compiled default `v1`), archive suffix, and `CCDNUM`. |
| Gaia or external catalog mismatch | Tile filenames/sky coverage and one-based raw column indices. |
| Missing science products | Required upstream stage disabled, or exposure-list entries point to another tree. |
| Compiler / runtime ABI errors | Use one MPI implementation and matching CFITSIO/FFTW/BLAS runtime libraries. |
| Container path not found | Validate `mounts` and `paths` in [PATHS.md](PATHS.md). |

For containerized execution, use [Docker](DOCKER.md) or [Slurm + Apptainer](HPC.md), rather than copying the native MPI command into the container runner.
