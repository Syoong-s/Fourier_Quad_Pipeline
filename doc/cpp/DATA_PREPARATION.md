# 🗂️ Data Preparation Guide — C++ Standard

[Home](../../README.md) · [Quick Start](CONFIGURATION.md) · [C++ User Guide](CPP_GUIDE.md) · [Parameters](CPP_PIPELINE_PARAMETERS.md) · [Container Paths](PATHS.md)

Prepare input data before invoking the C++ workflow. Native paths are filesystem paths; Docker/Apptainer paths must be visible **inside** the container and agree with `cpp_docker/project.json`.

## Overview of Required Inputs

| Input | Purpose | Requirement |
| :--- | :--- | :--- |
| Science FITS/FZ | Galaxy/source images and PSF measurements | Multi-extension archives matching the configured dataset prefix and `CCDNUM` convention. |
| DQ FITS/FZ | Pixel masks, default `include_mask = 2` | Matching archive naming, physical CCD identification. |
| Gaia astrometry tiles | Astrometric calibration and star matching | Sky coverage and filenames must match `astrometry_cat_type`. |
| External source catalog tiles | Source matching and photometric redshifts | Canonical one-degree tiles and known numeric column schema. |
| Exposure lists | Input for later phases when initialization is skipped | A top-level list of per-exposure lists; absolute chip paths should remain accessible. |

## Science Images & DQ Masks

### Science FITS/FZ archives

`process_init` discovers archives below `[init].science_root`, filters by the prefixes in `[init].datasets`, and optionally by the OR-matched, case-sensitive tokens in `[init].contains`. The compiled archive suffix is `.fits.fz`. The compiled `contains` default is `v1`, so **actively clear it** with `contains =` if that token is not present in your archive basenames.

An archive should contain image HDUs with usable `CCDNUM` headers. The extracted dataset tree uses the configured `output_root`, e.g.:

```text
/data/processed/gband/
  science/<exposure>/<exposure>_<sequence>.fits
  dqmask/<exposure>/<exposure>_<CCDNUM>.fits
  expolists/<exposure>.list
/data/processed/expo_gband.list
```

The Science suffix reflects an image HDU occurrence; the DQ suffix identifies the physical CCD. Do not assume those numbers are interchangeable. The initialization program associates DQ exposure stems with the compiled `ood` → `ooi` convention. A survey with different names or headers may require changes to `config/InitConfig.hpp` and a clean rebuild.

### Mask modes and checks

`[lensing].include_mask` selects the processing branch (`0`–`3`); the default value `2` requires correctly prepared pixel-quality input. Before a survey run, inspect a Science image and its paired DQ chip, confirm dimensions, and verify that catalog/source coordinates lie on the intended CCD.

## Gaia Astrometric Catalog

Set `[lensing].astrometry_cat` to the directory read by `process_main`, and choose the correct layout:

| `astrometry_cat_type` | Files | Purpose |
| :---: | :--- | :--- |
| `1` (default) | Large reference tiles such as `gaia_*.cat` | Original large-sky-tile indexing. |
| `2` | One-degree `astra_RA_..._Dec_....dat` tiles | Produced by the optional Gaia tile builder. |

Make sure the numeric columns and headers match the selected parser and that the tiles cover every exposure. To build the one-degree form, enable `run_process_astrocat = true`, set `[astrocat].input_directory` and `[astrocat].output_directory`, then separately set the **consumer** `[lensing].astrometry_cat` to the generated directory.

## External Source Catalog

The default canonical source row is **18 numeric columns**. Raw RA/Dec/ZP one-based defaults are `5`, `6`, `17`. The program expects a specific DES-style layout for the FD analysis: flags in columns 1–4 and magnitude positions 7, 9, 11, 13, 15 (one-based). These are **not** independently selectable through `mag_*_column` INI keys in this implementation.

The optional `process_extcat` phase reads raw source catalogs and publishes one-degree `extern_RA_..._Dec_....dat` files. The prefix comes from a compiled tile-naming constant. If the raw input has a different schema:

```ini
[extcat]
input_directory = /data/catalogs/source_raw
output_directory = /data/catalogs/source_tiles
use_explicit_coordinate_columns = true
ra_column = 5
dec_column = 6
zp_column = 17
total_columns = 18
```

`use_explicit_columns = true` with `input_columns = ...` enables **ordered raw-column projection**. Positions are one-based in that list. Keep output widths and coordinate mappings consistent with `process_main`, `process_rearr`, and `process_fd`. The FD reader also requires the original DES flag and magnitude fields; some projections are rejected when FD is enabled. See [External Catalog Configuration](CPP_PIPELINE_PARAMETERS.md#external-catalog-preparation-extcatconfig).

The effective external-tile directory is shared between `[extcat].output_directory` and `[lensing].source_cat`. Use the same mounted destination for producer and consumer.

## Existing Exposure Lists (Downstream-Only Runs)

If `run_process_init = false`, provide an existing `[process].expo_list` or allow derivation of `output_root/expo_<target>.list`. A top-level list points to per-exposure lists, which in turn hold chip FITS paths. When moving data to a different machine or into a container, update the **paths inside those files** as necessary; updating only the INI root is insufficient.

## Pre-Flight Checklist

- [ ] Science/DQ stems, `CCDNUM`, and images have been sampled and verified.
- [ ] `datasets` and `contains` select only intended archives.
- [ ] Gaia layout, tile names and sky coverage match the astrometry mode.
- [ ] External source columns, projection, and FD constraints have been checked.
- [ ] Output directory and each MPI rank have the required access permissions.
- [ ] A small pilot run produces valid exposure lists and representative FITS/catalog products.
