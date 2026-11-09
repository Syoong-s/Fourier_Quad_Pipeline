# 📜 C++ Third-Party Notices

[Home](../../README.md) · [C++ User Guide](CPP_GUIDE.md) · [MIT License](../../LICENSE)

Repository-authored source, launcher scripts and documentation are under the repository's [MIT License](../../LICENSE). External compilers, MPI implementations, scientific libraries, binary images and downloaded archives retain their own upstream licenses. Consult upstream license files when distributing Docker/OCI or Apptainer images.

The `cpp_docker/Dockerfile` installs a pinned C++ scientific stack using `pixi.toml` and `pixi.lock`, with other toolchain components built and validated from upstream archives. These **build-time files must remain present** if you build the image.

| Component category | Representative dependencies | Licensing note |
| :--- | :--- | :--- |
| Operating system and build tools | Rocky Linux 8.10, GCC 12.3.0, GNU Make, Pixi | Follow package and base-image notices. |
| MPI and scheduler interface | OpenMPI 4.1.8, Slurm PMI2 client 25.11.2 | Follow upstream BSD/GPL-family component terms. |
| FITS and Fourier transforms | CFITSIO 4.6.4, FFTW3/FFTW3f 3.3.11 | Respect upstream library and redistribution terms. |
| Scientific linear algebra | Eigen3, BLAS, LAPACK, OpenBLAS | Check exact conda/package license metadata. |
| Python and scripting | Python 3 standard library, shell utilities | Underlying runtime/tool licenses apply. |

An OCI image or SIF is a distribution of the included third-party binaries. This document is a summary, not an exhaustive inventory or substitute for upstream license notices. The package lock and pinned Docker build inputs are the best sources for the precise dependency set.
