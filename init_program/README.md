# 🗂️ Standalone Science/DQ Dataset Initializer

[Repository Home](../README.md) · [C++ Data Preparation](../doc/cpp/DATA_PREPARATION.md) · [Fortran77 Data Preparation](../doc/f77/DATA_PREPARATION.md)

`init_program/init_program.py` is a standalone Python implementation for extracting per-chip Science and data-quality (DQ) images from multi-extension `.fits.fz` archives and generating the exposure-list directory tree. It is independent of the scientific C++ and Fortran77 executables. It **never modifies the original archives**.

The script has embedded `#SBATCH` directives and supports local serial, MPI and direct Slurm batch submission. It can be used when preparing the same file tree for either scientific implementation; ensure the selected scientific program's data and catalog schema requirements are satisfied.

## Runtime Requirements

- Python **3.10+**.
- `astropy` and `numpy` for FITS handling.
- `mpi4py` built for the MPI runtime used on the host if launching multiple ranks.
- `srun` only when submitting to Slurm via the script's batch entry point.

For example, install the Python packages in an isolated environment with an MPI implementation compatible with `mpi4py`.

## Output Tree

The initializer writes under the selected output root:

```text
<output-root>/
├── <target>/
│   ├── science/<exposure>/<exposure>_<sequence>.fits
│   ├── dqmask/<exposure>/<exposure>_<CCDNUM>.fits
│   ├── expolists/<exposure>.list
│   ├── result/
│   ├── stamps/...
│   └── astrometry/...
├── expo_<target>.list
├── fits_<target>.list
└── init_<target>_manifest.json
```

Science outputs follow the image HDU occurrence sequence; DQ outputs use physical `CCDNUM`, not the Science sequence. The default DQ naming convention maps exposure stems from `ood` to `ooi`. If your data do not follow these conventions, inspect the initializer's command/options and the consumer's header rules before running.

The top-level `expo_<target>.list` is the required **positional argument** for the Fortran77 executable; the C++ pipeline can also use this list when running without its built-in `process_init` phase.

## Portable Commands

Inspect all command-line options:

```bash
python3 init_program/init_program.py --help
```

Prepare the data on one rank:

```bash
python3 init_program/init_program.py \
  --science-root /data/archive/science \
  --dq-root /data/archive/dq \
  --output-root /data/processed \
  --target gband --prefix c4d_ --contains v1 \
  --existing fail
```

Use MPI when appropriate:

```bash
mpirun -np 8 python3 init_program/init_program.py \
  --science-root /data/archive/science \
  --dq-root /data/archive/dq \
  --output-root /data/processed \
  --target gband --prefix c4d_ --contains v1 \
  --existing resume
```

Submit with Slurm if your environment supports direct batch submission of a Python script:

```bash
sbatch init_program/init_program.py \
  --science-root /data/archive/science \
  --dq-root /data/archive/dq \
  --output-root /data/processed \
  --target gband --prefix c4d_ --contains v1 \
  --existing resume
```

Review the script's embedded `#SBATCH` resources and site-specific environment first. `--no-srun` allows serial troubleshooting inside an allocation.

## Existing-Output Modes

| `--existing` value | Behavior |
| :--- | :--- |
| `fail` | Refuse to overwrite existing planned chip products. |
| `resume` | Reuse valid products, regenerate missing or invalid items. |
| `overwrite` | Regenerate outputs and replace them atomically. |

The initializer writes files through a staging area under `<target>/.fq_init_tmp/`, then atomically publishes chip outputs, lists, and a manifest on the same filesystem. Failed archives are recorded; a nonzero return status indicates that the complete requested initialization did not succeed. Successful previous outputs can be inspected or reused with `--existing resume`.

## Checks Before Scientific Processing

- Compare a sampled Science image and DQ mask by physical CCD identification.
- Inspect both exposure-list levels and confirm that every path is accessible to all MPI ranks.
- Confirm the selected token filter actually matches intended archive basenames.
- Check output permissions and space. Do not put generated outputs under the raw archive roots.
- For a containerized scientific run, verify that exposure-list **contents** use paths visible inside the container; host paths are not automatically rewritten by Docker or Apptainer.
