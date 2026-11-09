# C++ Standard Documentation

[Repository Home](../../README.md) · [Quick Start](CONFIGURATION.md)

This is the standalone documentation index for the **C++17 MPI implementation** in `cpp_Standard/`. Use the C++ program's optional `--config pipeline.ini` mechanism, and use **`cpp_docker/fqctl.py --json`** for JSON-driven Docker or HPC deployment.

| Topic | Document |
| :--- | :--- |
| Essential camera, dataset, path and workflow setup | [Quick Start](CONFIGURATION.md) |
| Compiler, MPI, INI syntax, scientific stages and run recipes | [C++ User Guide](CPP_GUIDE.md) |
| Complete INI key reference and compiled-only parameter groups | [C++ Full Parameter Reference](CPP_PIPELINE_PARAMETERS.md) |
| FITS/DQ, Gaia and external source catalogs | [Data Preparation](DATA_PREPARATION.md) |
| Desktop Docker JSON→INI launcher | [Docker Workstation](DOCKER.md) |
| Slurm + Apptainer execution | [HPC Runner](HPC.md) |
| Mounts, container paths and producer/consumer aliases | [Container Path Reference](PATHS.md) |
| Dependency and redistribution notices | [Third-Party Notices](THIRD_PARTY_NOTICES.md) |

The standalone [archive initializer](../../init_program/README.md) is an optional preprocessing tool. Scientific code and parameter defaults live under `cpp_Standard/`; the documentation does not replace those source-of-truth values.
