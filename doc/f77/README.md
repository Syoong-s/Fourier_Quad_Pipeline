# Fortran77 Documentation

[Repository Home](../../README.md) · [Quick Start](CONFIGURATION.md)

This is the standalone documentation index for the **Fortran77 MPI implementation** in `f77/`. This program uses compile-time `.inc` configuration and a required **positional exposure-list argument**. Its Docker/HPC launcher is **`f77_docker/fqctl.py --project`**; there is no runtime INI parser.

| Topic | Document |
| :--- | :--- |
| Essential camera, site, catalog and compiled path settings | [Quick Start](CONFIGURATION.md) |
| Native compiler, MPI, stage scheduling and run recipes | [Fortran77 User Guide](F77_GUIDE.md) |
| Compile-time include constants and scientific parameter families | [F77 Parameter Reference](F77_PIPELINE_PARAMETERS.md) |
| Chip FITS, large Gaia tiles and exposure-list hierarchy | [Data Preparation](DATA_PREPARATION.md) |
| Desktop Docker project and isolated source build | [Docker Workstation](DOCKER.md) |
| Slurm + Apptainer PMI2 execution | [HPC Runner](HPC.md) |
| Data mounts, staged `para.inc` and exposure-list paths | [Container Path Reference](PATHS.md) |
| Compiler and scientific-library licensing | [Third-Party Notices](THIRD_PARTY_NOTICES.md) |

To extract Science/DQ images and prepare top-level exposure lists, see the [standalone dataset initializer](../../init_program/README.md).
