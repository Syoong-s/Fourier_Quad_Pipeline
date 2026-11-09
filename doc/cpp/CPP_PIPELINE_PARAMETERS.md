# 📋 C++ Pipeline Parameter Reference

[Home](../../README.md) · [C++ User Guide](CPP_GUIDE.md) · [Quick Start](CONFIGURATION.md) · [Data Preparation](DATA_PREPARATION.md) · [Container Paths](PATHS.md)

This reference covers every key shown in `cpp_Standard/pipeline.example.ini` and the principal compiled-only parameter families. The scientifically authoritative defaults are defined in `cpp_Standard/config/*.hpp`; the optional INI is a **per-run override**, not a second independent set of defaults.

## How to Use This Reference

- To use compiled defaults, omit a key (or leave its sample line commented with `#`).
- To override an INI-supported field, supply an active `[section]` plus `key = value` and pass `--config path/to/file.ini`.
- An empty scalar value is an error; an intentionally empty list or string may clear that field.
- `LensingConfig` scientific options listed in the INI are propagated to the scientific consumers on **each MPI rank**. Other numerical constants remain compile-time only.
- Changes to `.hpp` headers require recompilation; changes to INI values do not.

The reference tables below describe **supported syntax**, not recommendations to change scientific branches. For a new survey, begin with [Configuration Quick Start](CONFIGURATION.md).

## Quick Jump

| Section | Topic |
| :--- | :--- |
| [`[process]`](#workflow-processconfig) | Workflow switches and output paths |
| [`[init]`](#archive-initialization-initconfig) | Science/DQ data and dataset rules |
| [`[astrocat]`](#gaia-preparation-astrocatconfig) | Gaia tile producer |
| [`[extcat]`](#external-catalog-preparation-extcatconfig) | External source catalog schema |
| [`[lensing]`](#branches-camera-geometry-and-catalogcalibration-paths-lensingconfig) | Geometry, PSF/algorithm switches, input paths |
| [Compiled-only](#compiled-only-scientific-settings) | Internal constants and derived layouts |

## Workflow (`ProcessConfig`)

| INI key | Meaning / constraints |
| :--- | :--- |
| `expo_list` | Existing exposure-list file for downstream-only runs; empty derives from output_root. |
| `rearr_output_directory` | Rearranged catalog directory; relative to rearr_output_base_directory or the dataset root. |
| `rearr_output_base_directory` | Rearranged output base; empty uses the dataset root. |
| `rearranged_expo_list_filename` | Rearranged exposure-list filename, not a directory. |
| `rearranged_expo_list_directory` | Rearranged list directory; empty uses the input-list parent. |
| `fd_expo_list` | FD exposure-list file; empty uses the rearranged list. |
| `fd_output_directory` | FD output directory name or absolute directory. |
| `fd_output_base_directory` | FD output base; empty uses the dataset root. |
| `run_process_astrocat` | Repartition raw Gaia catalogs into one-degree tiles. [true / false] |
| `run_process_extcat` | Repartition raw external source catalogs. [true / false] |
| `run_process_init` | Extract Science/DQ archives and publish datasets and exposure lists. [true / false] |
| `run_process_main` | Run the numerical measurement phase. [true / false] |
| `run_process_rearr` | Spatially rearrange the main-phase catalogs. [true / false] |
| `run_process_fd` | Run scientific field-distortion measurement; not a container diagnostic. [true / false] |

**Compiled defaults:** catalog tilers disabled; `run_process_init`, `run_process_main`, and `run_process_rearr` enabled; `run_process_fd` disabled. The workflow order is astrocat → extcat → init → main → rearr → FD. Paths for rearrangement and FD are optional overrides.

## Archive initialization (`InitConfig`)

| INI key | Meaning / constraints |
| :--- | :--- |
| `science_root` | Raw Science FITS/FZ archive root. |
| `dq_root` | Raw DQ FITS/FZ archive root. |
| `output_root` | Dataset output root; science, dqmask, stamps, result, astrometry and expolists share this mount. |
| `datasets` | Comma-separated TARGET:PREFIX pairs, e.g. gband:c4d_; replaces the old dataset/target/prefix options. [e.g. gband:c4d_] |
| `contains` | Comma-separated archive basename tokens (OR); an active empty value disables filtering. [comma-separated] |
| `existing` | Existing dataset policy. [fail\|resume\|overwrite] |
| `f77_max_path` | Maximum generated path length; 0 disables the guard. [non-negative integer] |

**Compiled defaults:** one dataset `gband:c4d_`, `contains = v1`, archive suffix `.fits.fz`, image-identity header `CCDNUM`, DQ stem replacement `ood` → `ooi`, and `existing = fail`. These archive-format conventions are **not** INI keys and must be changed in `InitConfig.hpp` for a different instrument/archive.

## Gaia preparation (`AstroCatConfig`)

| INI key | Meaning / constraints |
| :--- | :--- |
| `input_directory` | Flat directory of raw Gaia catalogs beginning with RA/Dec columns. |
| `output_directory` | Published Gaia tiles; usually share one mount with lensing.astrometry_cat. |
| `add_header` | true: input starts with data; false: skip exactly one input header line. [true / false] |
| `existing_policy` | Existing Gaia tile policy. [fail\|overwrite] |

The producer `output_directory` and the consumer `[lensing].astrometry_cat` are independent runtime locations. Match them explicitly when consuming newly prepared Gaia tiles.

## External catalog preparation (`ExtCatConfig`)

| INI key | Meaning / constraints |
| :--- | :--- |
| `input_directory` | Raw external source-catalog directory. |
| `output_directory` | Published external source-catalog tiles. |
| `filename_tokens` | Comma-separated basename tokens (OR); empty selects all files. [comma-separated] |
| `recursive` | Search raw catalog subdirectories recursively. [true / false] |
| `delimiter` | Input column delimiter. [auto\|whitespace\|comma\|tab] |
| `header_mode` | Input header handling. [auto\|present\|absent] |
| `malformed_policy` | Malformed-row policy. [fail\|skip] |
| `existing_policy` | Existing external tile policy. [fail\|overwrite] |
| `chunk_mib` | Positive MPI byte-range task size in MiB. [positive integer] |
| `total_columns` | External catalog pass-through width; also controls rearr/FD column offsets. [positive integer] |
| `use_explicit_columns` | Set true to use the ordered input_columns projection. [true / false] |
| `input_columns` | Comma-separated one-based input column indices in output order. [e.g. 1,2,3] |
| `use_explicit_coordinate_columns` | Set true to use explicit RA/Dec positions rather than header discovery. [true / false] |
| `ra_column` | One-based RA column in the raw input (also used by FD). [positive integer] |
| `dec_column` | One-based Dec column in the raw input (also used by FD). [positive integer] |
| `zp_column` | One-based zp/photo-z column; FD also expects DES flag/magnitude fields. [positive integer] |

The raw one-based coordinate defaults are RA `5`, Dec `6`, ZP `17`; pass-through row width is `18`. The optional `input_columns` list is an ordered projection of raw columns, not a list of output positions. The C++ implementation does **not** expose separate `mag_g_column`, `mag_r_column`, `mag_i_column`, `mag_z_column`, or `mag_y_column` INI keys. The FD reader still depends on the fixed DES flags/magnitude raw positions (1–4 and 7/9/11/13/15); only supported projections can be used for FD.

## Branches, camera geometry and catalog/calibration paths (`LensingConfig`)

| INI key | Meaning / constraints |
| :--- | :--- |
| `astrometry_cat` | Gaia tile directory read by the main phase. |
| `source_cat` | Source tiles: aliases extcat.output_directory for this executable. |
| `flat_path` | Directory containing super-flat FITS images. |
| `psf_path` | External PSF FITS root. |
| `process_stage` | Scientific settings below are now applied at runtime on every MPI rank. Product of enabled stage primes. Factor 23 requires factor 19. [positive integer] |
| `ccd_split` | CCD partition: 1 for the whole CCD, 2 for amplifier halves. |
| `gal_smooth` | Galaxy-stamp smoothing selector; non-negative. [non-negative integer] |
| `star_smooth` | Star-stamp smoothing selector; non-negative. [non-negative integer] |
| `pixel_size` | Pixel scale in arcsec/pixel; positive. [positive number] |
| `nmax_chip` | Positive CCD count. [positive integer] |
| `chipnx` | Physical CCD width in pixels; positive. [positive integer] |
| `chipny` | Physical CCD height in pixels; positive. [positive integer] |
| `astrometry_trivial` | Astrometry: 0 for Gaia, 1 for identity mapping. |
| `astrometry_cat_type` | Gaia layout: 1 for legacy large tiles, 2 for one-degree tiles. |
| `include_flat` | Super-flat correction: 0 off, 1 on. |
| `include_mask` | Mask mode: 0, 1, 2 or 3. |
| `ext_cat` | External sources: 0 off, 1 on; FD requires 1. |
| `ext_psf` | PSF input: 0 for exposure stars, 1 for external PSF. |
| `deblending` | Deblending: 0 off, 1 on. |
| `psf_type` | PSF model: 1 local polynomial, 2 hybrid. |
| `psf_ms` | PCA/multi-scale PSF reconstruction: 0 off, 1 on. |

**Compiled DECam-like defaults:** `pixel_size = 0.2628`, `nmax_chip = 62`, `chipnx = 2046`, `chipny = 4094`, `ccd_split = 2`, `astrometry_trivial = 0`, `astrometry_cat_type = 1`, `include_flat = 0`, `include_mask = 2`, `ext_cat = 1`, `ext_psf = 0`, `deblending = 1`, `psf_type = 1`, `psf_ms = 0`. Check real FITS detector geometry before changing the camera fields.

## Compiled-only scientific settings

All other `config/*.hpp` parameters are compiled constants. They are not valid INI keys, even if their names resemble a runtime override. The source code documents the exact current value, units, buffer assumptions and scientific role.

| Header | Important compiled-only families | What requires review |
| :--- | :--- | :--- |
| `config/LensingConfig.hpp` | `saturation_thresh`, `PreprocsType`, `include_BGsub`, `psf_order`, `npo`, `npl`, `nstar_min`, grouping/selection thresholds | Image calibration, PSF-star selection, background/noise estimators |
| `config/LensingConfig.hpp` | `ns`, `nsns`, `chip_margin`, `flag_thresh`, `source_thresh`, `core_thresh`, `blocksize`, noise sigma/covariance controls | Stamp geometry, detection thresholds and noise model |
| `config/LensingConfig.hpp` | `npara`, `len_g`, `len_s`, `npd`, `i*` catalog index definitions | Fixed scientific catalog layout; not general camera settings |
| `config/InitConfig.hpp` | `ARCHIVE_SUFFIX`, `CCDNUM_KEYWORD`, `DQ_STEM_REPLACE_FROM`, `DQ_STEM_REPLACE_TO` | Input FITS/FZ and DQ naming contracts |
| `config/pathconfig.hpp` | `ASTROMETRY_TILE_PREFIX`, `SOURCE_CAT_TILE_PREFIX`, subcatalog and product-directory names | Tile file discovery and generated output naming |
| `config/ProcessRearrConfig.hpp` | Spatial-grid extent, precision, subcatalog partition sizes and derived layout | Spatial partitioning and catalog output format |
| `config/FDConfig.hpp` | FD estimator mode, binning, jackknife, source and star cuts, compiled DES fields | Scientific interpretation of the FD estimator |

### Runtime selection and coupled values

`cpp_Standard/include/RuntimeScientificOverrides.hpp` provides mutable shadows of specified compiled constants; `src/RuntimeScientificOverrides.cpp` applies validated INI values to each MPI process. The factory still loads original compiled defaults from configuration headers.

The external row format has coupled widths and offsets. With `use_explicit_columns = true`, the effective external row width is the length of `input_columns` instead of the original `total_columns`; a rearranged row contains that width plus exposure/CCD and measurement fields. For FD, keep the effective width and column mapping consistent with the actual projected catalog. The supplied FD implementation imposes additional DES-field requirements; an unsupported projection must be rejected, not guessed.

The workflow's nine numerical stage switches are controlled by prime factors of `[lensing].process_stage`: `2, 3, 5, 7, 11, 13, 17, 19, 23`. Factor `23` requires factor `19`. Scientific stage choices must remain consistent with the intermediate files present on disk.

**Source of truth:** inspect `cpp_Standard/pipeline.example.ini` for the exact INI spelling, `src/RuntimeConfig.cpp` for validation, and the relevant `config/*.hpp` file for the compiled default and any scientific invariants.
