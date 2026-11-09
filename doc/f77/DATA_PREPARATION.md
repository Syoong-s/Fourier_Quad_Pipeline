# 🗂️ Data Preparation Guide — Fortran77

[Home](../../README.md) · [F77 User Guide](F77_GUIDE.md) · [Configuration](CONFIGURATION.md) · [Container Paths](PATHS.md)

The F77 executable consumes a **prepared exposure-list tree**, not raw Science `.fits.fz` files as command-line arguments. The standalone [`init_program.py`](../../init_program/README.md) can extract Science and DQ CCD images without changing the source archives.

## Required Inputs

| Input class | Purpose | Format / requirement |
| :--- | :--- | :--- |
| Science CCD FITS | Image processing, stamps, PSF and galaxy shear | Per-CCD FITS files with expected physical geometry. |
| DQ CCD FITS | Bad-pixel and mask handling | Paired by CCD identity; required by default `include_Mask=2`. |
| Gaia astrometry | Star matching and WCS | Compiled large-tile parser with header and numeric sky coordinates. |
| External source tiles | Photometric redshift and source matching | Canonical one-degree tiles with expected compiled numeric columns. |
| Top-level exposure list | Primary F77 CLI argument | Entries naming per-exposure list files. |
| Per-exposure list | Input for scientific stages | Entries naming per-chip Science FITS files. |

## Science and DQ Archive Preparation

The standalone initializer accepts Science and DQ archives with suffix `.fits.fz`, identifies CCDs via `CCDNUM`, and writes extracted chip files beneath an output dataset. Use your actual survey archive naming conventions for the archive filter; the default DQ stem mapping follows `ood` → `ooi`.

Example output layout:

```text
/data/processed/
├── gband/
│   ├── science/<exposure>/<exposure>_<sequence>.fits
│   ├── dqmask/<exposure>/<exposure>_<CCDNUM>.fits
│   ├── expolists/<exposure>.list
│   ├── stamps/...
│   ├── astrometry/...
│   └── result/...
├── expo_gband.list
├── fits_gband.list
└── init_gband_manifest.json
```

Sample input:

```bash
python3 init_program/init_program.py \
  --science-root /data/archive/science \
  --dq-root /data/archive/dq \
  --output-root /data/processed \
  --target gband --prefix c4d_ --contains v1 \
  --existing fail
```

The initializer can be run under MPI or Slurm; see its [full guide](../../init_program/README.md). The same initialized dataset may be consumed by either scientific executable when their input and catalog contracts are satisfied.

## Gaia Astrometric Reference

The F77 astrometry reader expects the original **large sky-tile** layout, for example `gaia_p1_00.cat`, with a header row and numeric RA/Dec columns. Point `ASTROMETRY_CAT` in `f77/para.inc` to the correct directory before building. Coverage must contain all survey exposures. Do not give it one-degree `astra_...dat` tiles without verifying the compiled parser and naming logic.

## External Source Catalog

The compiled external catalog convention uses `SOURCE_CAT` and a one-degree prefix `extern_`. A typical tile basename is:

```text
extern_RA_123_124_Dec_m05_m04.dat
```

Each row must follow the compiled field-index layout; it is **not** configurable by a runtime INI. Inspect the catalog header, coordinate columns, photometric redshift, and magnitude fields before running with a different survey. A catalog with the right filename but the wrong columns can yield scientifically invalid matches.

## Exposure Lists and Processing Tree

The top-level list is provided as the **first positional argument** to `Fourier_Quad_Pipe`. Each item points to a per-exposure list; each per-exposure list points to chip Science FITS images. Make sure list paths are absolute or resolvable by all ranks and that referenced output directories can be written.

The initializer uses a two-stage exposure-list structure and publishes list/manifest files atomically. Do not confuse the `expo_gband.list` top-level list with the chip-level list for a single exposure.

## Pre-Flight Checklist

- [ ] FITS headers, Science dimensions, and `CCDNUM` values agree with the compiled camera constants.
- [ ] Every listed Science chip has the expected DQ mask when required.
- [ ] Large Gaia tiles cover the target footprint and have expected header/columns.
- [ ] One-degree external tiles follow the compiled filename and column contract.
- [ ] Exposure-list paths are visible to all MPI processes (and inside any container).
- [ ] Writable result directories are available, with enough free storage.
- [ ] Small pilot outputs were inspected before survey-scale analysis.
