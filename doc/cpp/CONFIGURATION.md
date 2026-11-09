# Configuration Quick Start

[Home](../../README.md) · [Data Preparation](DATA_PREPARATION.md) · [C++ User Guide](CPP_GUIDE.md) · [Full Parameter Reference](CPP_PIPELINE_PARAMETERS.md)

This guide covers **site-, camera-, and dataset-specific settings** that a new user should review before running `cpp_Standard/Fourier_Quad_Pipe`. It does not teach numerical tuning, alternative PSF algorithms, estimator choices, or internal science branches.

The values in the examples are illustrative, **not a validated configuration for a new instrument**. Test on a small representative sample before processing a survey.

## 1. Create a run configuration

The pipeline combines compiled defaults in `cpp_Standard/config/*.hpp` with optional INI overrides.

```bash
cd cpp_Standard
cp pipeline.example.ini pipeline.ini
```

The example INI contains mostly commented entries. **Copying it without changing the paths does not make it suitable for your site:** the executable otherwise retains the repository's compiled site-specific paths.

| Entry | Meaning |
| --- | --- |
| Omitted key or `# key =` | Keep the compiled default from `config/*.hpp`. |
| `key = value` | Override the compiled default. |
| Active empty numerical/Boolean key, e.g. `pixel_size =` | Configuration error; it does not restore the default. |
| Active empty path or list | May clear that setting; only use where this is intentional. |

The executable accepts `--config PATH` or `--config=PATH`. Without it, the executable uses compiled defaults and does **not** automatically locate `pipeline.ini`. Editing the INI does not require recompilation. Editing a `config/*.hpp` header does.

## 2. Identify the input data and output locations

Prepare the source files following the [Data Preparation Guide](DATA_PREPARATION.md).

| INI setting | What to provide |
| --- | --- |
| `[init].science_root` | Directory containing raw Science FITS/FZ archives. |
| `[init].dq_root` | Directory containing corresponding DQ FITS/FZ archives, when the selected mask mode needs them (the default does). |
| `[init].output_root` | Writable parent directory for extracted datasets and all generated science products. |
| `[init].datasets` | Comma-separated `TARGET:PREFIX` pairs, e.g. `gband:c4d_`. The target names the dataset output folder; the prefix matches archive basenames. |
| `[init].contains` | Case-sensitive archive basename tokens, OR-matched. **Review for every dataset**: the compiled default is `v1`. |
| `[lensing].astrometry_cat` | Directory of prepared Gaia reference tiles consumed by the main pipeline. |
| `[lensing].source_cat` | Directory of prepared canonical external source-catalog tiles. |

For example:

```ini
[init]
science_root = /data/survey/science
dq_root = /data/survey/dq
output_root = /data/survey/processed
datasets = gband:c4d_
contains =

[lensing]
astrometry_cat = /data/catalogs/gaia_tiles
source_cat = /data/catalogs/source_tiles
```

Replace the example paths and dataset pair with actual values. Here `contains =` intentionally **disables** the compiled `v1` archive-name filter. If a filter is needed, specify your dataset's actual token instead.

Generated `science/`, `dqmask/`, `stamps/`, `expolists/`, `result/`, and other dataset product subdirectories normally live beneath the output tree; they do not require independent mounts or path settings.

## 3. Review camera-specific geometry

The compiled defaults assume DECam. Verify the following values against the physical detector and extracted FITS images:

| INI setting | Compiled default | When to change |
| --- | --- | --- |
| `[lensing].pixel_size` | `0.2628` arcsec/pixel | Different angular pixel scale. |
| `[lensing].nmax_chip` | `62` | Different physical CCD count. |
| `[lensing].chipnx` | `2046` pixels | Different CCD width. |
| `[lensing].chipny` | `4094` pixels | Different CCD height. |
| `[lensing].ccd_split` | `2` | Whole CCD (`1`) rather than two amplifier halves (`2`). |

For illustration **only**, a verified 16-CCD camera might need different values:

```ini
[lensing]
pixel_size = 0.30
nmax_chip = 16
chipnx = 4096
chipny = 4096
ccd_split = 1
```

**Do not use these illustrative values without checking your instrument.** Stage 1 reads FITS image dimensions, but downstream geometry-dependent operations also use the configured values. A different detector may require code-level validation beyond changes to these five settings.

Input-format conventions that are **compile-time only** also need review when changing surveys:

| Header | Setting | Current convention |
| --- | --- | --- |
| `config/InitConfig.hpp` | `ARCHIVE_SUFFIX` | `.fits.fz` |
| `config/InitConfig.hpp` | `CCDNUM_KEYWORD` | `CCDNUM` identifies physical CCDs |
| `config/InitConfig.hpp` | `DQ_STEM_REPLACE_FROM` / `DQ_STEM_REPLACE_TO` | `ood` → `ooi` |
| `config/pathconfig.hpp` | `ASTROMETRY_TILE_PREFIX` / `SOURCE_CAT_TILE_PREFIX` | `astra_` / `extern_` for one-degree tile files |

Edit these headers only if the actual archive format or naming contract differs, then run `make clean && make -j4`. Do not modify derived catalog indices or scientific constants to compensate for unexpected data formats.

## 4. Match your reference catalogs

**Gaia tiles:** `[lensing].astrometry_cat_type = 1` (default) expects the legacy large `gaia_*.cat` layout. Value `2` expects one-degree `astra_RA_..._Dec_....dat` tiles. Set the value according to the files actually present, not the type of camera. Ensure Gaia sky coverage includes all science exposures. See [Gaia data preparation](DATA_PREPARATION.md#gaia-astrometric-catalog).

**External source catalog:** canonical tiles must include RA, Dec, photometric redshift (`zp`), and at least one observed-band magnitude. In this C++ version the magnitude indices consumed by the FD reader are compiled DES-layout positions; the INI does not expose separate `mag_*_column` keys. If the source survey has a different column schema, check the following one-based positions:

| INI key in `[extcat]` | What to verify |
| --- | --- |
| `total_columns` | Width of each pass-through catalog row; compiled default `18`. |
| `ra_column`, `dec_column`, `zp_column` | Raw one-based field indices; compiled defaults `5`, `6`, `17`. |
| `mag_g_column`, `mag_r_column`, `mag_i_column`, `mag_z_column`, `mag_y_column` | Raw magnitude indices; use `0` for a missing band. |
| `use_explicit_columns`, `input_columns` | Set together only for an ordered raw-column projection. |
| `use_explicit_coordinate_columns` | Enable if explicit RA/Dec indices should replace header discovery. |

Inspect [External Catalog Preparation](DATA_PREPARATION.md#external-source-catalog) and the [complete parameter reference](CPP_PIPELINE_PARAMETERS.md#external-catalog-preparation-extcatconfig) before adopting a different schema. Do not copy column positions from another survey without verifying them.

**Creating tiles from raw inputs:** if prepared tiles are unavailable, enable `[process].run_process_astrocat` and/or `run_process_extcat` and configure the corresponding `[astrocat].input_directory` / `output_directory` and `[extcat].input_directory` / `output_directory`. Raw input and tile output must be separate directories.

The Gaia producer path `[astrocat].output_directory` and the main consumer `[lensing].astrometry_cat` are **independent at runtime**: specify both if the main phase should consume newly generated Gaia tiles. Conversely, `[extcat].output_directory` and `[lensing].source_cat` refer to the same effective external-tile path in runtime configuration.

## 5. Select only the workflow phases you need

Compiled defaults enable `process_init`, `process_main`, and `process_rearr`; `process_fd` and catalog tiling are disabled. For a first pilot that initializes exposures and runs the main measurement using already prepared catalogs, set:

```ini
[process]
run_process_astrocat = false
run_process_extcat = false
run_process_init = true
run_process_main = true
run_process_rearr = false
run_process_fd = false
```

These switches select **top-level workflow steps**, not numerical algorithms. Enable the rearrangement and field-distortion phases once their prerequisites have been checked.

When using existing exposures rather than initializing new FITS archives, set `run_process_init = false`. Specify `[process].expo_list` when necessary; otherwise the program attempts to derive `<output_root>/expo_<target>.list`. Paths inside existing exposure lists must remain valid after moving data.

For ordinary runs, the configured `output_root` is normally sufficient. `rearr_output_*`, `rearranged_expo_list_*`, and `fd_output_*` are advanced workflow-path overrides, not required individually for a new dataset.

## 6. Build and validate a small run

```bash
cd cpp_Standard
make -j4
mpirun -np 4 ./Fourier_Quad_Pipe --config pipeline.ini
```

If a compile-time header was changed:

```bash
make clean
make -j4
```
## Checklist

Use the MPI launcher and rank count supported by your site. Before scaling up, check:

- [ ] Science/DQ archive names and physical `CCDNUM` values agree.
- [ ] `datasets` and `contains` select the intended exposures.
- [ ] The output directory is writable and has sufficient storage.
- [ ] Gaia layout, filenames, and sky coverage agree.
- [ ] External source columns and available magnitude bands match the catalog.
- [ ] Pixel scale, CCD geometry, and amplifier partition were verified.
- [ ] The pilot enables only the intended workflow phases.
- [ ] Generated exposure lists and representative catalogs were inspected.

For full build instructions, stage execution, and troubleshooting, see the [C++ User Guide](CPP_GUIDE.md). For every advanced parameter, see the [Full Parameter Reference](CPP_PIPELINE_PARAMETERS.md). New users generally should not change noise estimators, PSF model branches, scientific cuts, field-distortion statistics, or internally derived catalog fields.

For Docker and Apptainer, the same INI rules apply, but INI paths must be **container-visible paths** matching the JSON mount configuration; see [Docker](DOCKER.md) and [Container Paths](PATHS.md).
