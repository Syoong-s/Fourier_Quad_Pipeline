# 📜 Fortran77 Third-Party Notices

[Home](../../README.md) · [F77 User Guide](F77_GUIDE.md) · [MIT License](../../LICENSE)

The repository's scientific Fortran sources and deployment scripts are covered by the [MIT License](../../LICENSE). All externally downloaded tools and libraries retain their individual upstream licenses and redistribution obligations.

The `f77_docker/Dockerfile` builds a historical `linux/amd64` scientific stack from verified upstream archives. Representative components:

| Component | Reference version | Role |
| :--- | :--- | :--- |
| GNU GCC/GFortran | 4.8.5 | Historical Fortran compiler and runtime |
| MPICH | 4.1.2 | MPI execution and linking |
| CFITSIO | 4.3.1 | FITS input/output |
| LAPACK / reference BLAS | 3.8.0 | Numerical linear algebra |
| FFTPACK | Bundled with scientific source | Fourier transform routines |
| Rocky Linux | 8.10 | Base operating system |

The Dockerfile pins and verifies downloaded archives and includes a GNU 4.8.5 compatibility patch and scientific stack build files. These external projects are not relicensed by the repository's MIT license. If you distribute a Docker/OCI image or an Apptainer SIF, preserve the appropriate upstream notices and licenses for its included binaries.

An image is a **toolchain/runtime distribution**, not a distribution of private survey data or the scientific source tree. Consult the Dockerfile's exact archive versions and checksums when producing a licensing inventory.
