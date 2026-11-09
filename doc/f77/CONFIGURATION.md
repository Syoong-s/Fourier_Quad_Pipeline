# 📋 Essential Configuration — Fortran77

[Home](../../README.md) · [F77 User Guide](F77_GUIDE.md) · [Data Preparation](DATA_PREPARATION.md) · [Full Parameters](F77_PIPELINE_PARAMETERS.md)

Use this guide for the values that normally differ by **camera, site or dataset**. The scientific branches and internal numerical thresholds are already compiled into the Fortran77 executable. Unlike the C++ program, **the F77 executable does not parse an INI**; at execution it requires a top-level exposure-list pathname as its first positional argument.

## 1. Prepare the Dataset Tree

Prepare per-chip Science images, matching DQ masks, Gaia tiles, external source catalog tiles, and two levels of exposure-list files. The optional standalone [dataset initializer](../../init_program/README.md) can create the chip tree from raw multi-extension Science/DQ FITS/FZ archives.

Example top-level exposure list:

```text
"/data/processed/gband/expolists/exposure_001.list" 2
```

A per-exposure list then contains chip Science FITS paths. The original raw `.fits.fz` archives are **not** valid substitutes for the exposure-list argument. All referenced files must be visible to every MPI rank.

## 2. Review Site and Camera Constants

Edit these values **before** compiling `f77/`:

| File | Parameter | Compiled value / action |
| :--- | :--- | :--- |
| `f77/para.inc` | `ASTROMETRY_CAT` | Directory holding large Gaia reference tiles; replace site-specific paths. |
| `f77/para.inc` | `SOURCE_CAT` | Directory containing canonical external source tiles; replace site-specific paths. |
| `f77/para.inc` | `FLAT_PATH`, `PSF_PATH` | Calibration paths used only if corresponding branches are enabled. |
| `f77/para.inc` | `pixel_size` | `0.2628` arcsec/pixel; confirm the camera scale. |
| `f77/para.inc` | `npx`, `npy` | `3000`, `5000`: compiled Science image array bounds. |
| `f77/para.inc` | `CCD_split` | `2`: two amplifier regions per CCD. |
| `f77/cust_para.inc` | `chipnx`, `chipny` | `2046`, `4094`: physical CCD dimensions. |
| `f77/cust_para.inc` | `Camera_ccd_num` | `62`: expected CCD count. |
| `f77/para.inc` | `SOURCE_CAT_TILE_PREFIX` | `extern_`: canonical one-degree source tile prefix. |
| `f77/para.inc` | `NMAX_CHIP`, `NMAX_EXPO` | Check upper bounds for the camera and total exposure count. |

**Camera compatibility is not guaranteed by changing numbers alone.** Detector geometry, FITS ordering/CCD headers, fixed-size array capacities, and compiled catalog layout must be checked together. For unfamiliar instruments, make a small validation run before any survey analysis.

## 3. Verify Data and Catalog Conventions

- **Gaia astrometry:** F77 consumes the large-tile layout, typically files such as `gaia_p1_00.cat`, containing a header and numeric RA/Dec fields. Check coverage and naming in [Data Preparation](DATA_PREPARATION.md#gaia-astrometric-reference).
- **External source tiles:** Canonical `extern_RA_..._Dec_....dat` files must contain the columns expected by the compiled field indices, including usable coordinates, magnitude bands and redshift.
- **DQ masks:** The default `include_Mask = 2` requires valid physical-CCD masks in the prepared dataset tree.
- **Exposure lists:** Paths in the top-level and per-exposure lists must be valid on each rank; for containers, they must be **container-visible** paths.

## 4. Leave Scientific Branches Unchanged Initially

`PROCESS_stage = 2*3*5*7*11*13*17*19*23` selects all nine stages by default. The compiled selectors `include_FLAT`, `include_Mask`, `ext_cat`, `ext_PSF`, `PSF_type`, `PSF_Ms`, and the noise/PSF threshold constants are scientific controls, not generic survey-directory settings. Change them only when the corresponding method and prerequisite products have been validated.

## 5. Build and Check a Small Run

From the repository root, after editing include files:

```bash
make -C f77 clean
make -C f77 LAPACK_LIB_DIR=/path/to/lib \
  CFITSIO_LIB_DIR=/path/to/lib CFITSIO_LIB=/path/to/lib/libcfitsio.so
mpirun -np 4 ./f77/Fourier_Quad_Pipe /absolute/path/to/expo_gband.list
```

Use the compiler/MPI and library paths appropriate for the host. An include-file change always requires rebuilding the executable. Validate source and mask dimensions, star/catalog coverage, output permissions, and representative shear results before scaling up.

The optional [F77 JSON container launcher](DOCKER.md) creates an isolated, path-adjusted source snapshot, leaving the checked-in scientific includes unchanged. It does **not** turn the F77 program into an INI-configured executable.
