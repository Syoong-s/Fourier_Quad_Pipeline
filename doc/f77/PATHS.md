# 🔗 Fortran77 Container Path Reference

[Home](../../README.md) · [Docker](DOCKER.md) · [HPC](HPC.md) · [F77 Parameters](F77_PIPELINE_PARAMETERS.md)

The F77 program sees only paths in its runtime filesystem. Its reference and calibration locations are **compiled `PARAMETER` constants**, while the top-level exposure-list pathname is **the required positional argument**. `f77_docker/project.json` maps both to Docker or Apptainer mounts.

## How `mounts` and `paths` Interact

`mounts` maps host paths into container destinations. `paths` names the destinations used in staged `para.inc` and by the executable. More than one `paths` value can be placed below a shared parent mount. The launcher checks that all non-null paths are covered by a mount and that `exposure_list` is a real file.

```json
{
  "mounts": [
    {"host": "/survey/processed", "container": "/data/processed", "mode": "rw"},
    {"host": "/survey/catalogs", "container": "/data/catalogs", "mode": "ro"}
  ],
  "paths": {
    "astrometry_cat": "/data/catalogs/gaia",
    "source_cat": "/data/catalogs/source",
    "flat_path": null,
    "psf_path": null,
    "exposure_list": "/data/processed/expo_gband.list"
  }
}
```

These are excerpts from the complete JSON document described in [Docker](DOCKER.md). The generated staged source refers to **`/data/catalogs/gaia`**, not `/survey/catalogs/gaia`.

## Every Supported F77 Path Key

| JSON `paths` key | F77 consumer | Required? |
| :--- | :--- | :--- |
| `astrometry_cat` | `ASTROMETRY_CAT` in staged `para.inc` | Needed for Gaia astrometry unless a compatible alternative branch is chosen |
| `source_cat` | `SOURCE_CAT` in staged `para.inc` | Needed when the external source catalog branch is active |
| `flat_path` | `FLAT_PATH` in staged `para.inc` | Only when the flat calibration branch is enabled |
| `psf_path` | `PSF_PATH` in staged `para.inc` | Only for externally supplied PSF data |
| `exposure_list` | Required first positional argument to F77 `main.f` | **Yes** |

`null` means “do not override this compiled path” for the four `PARAMETER` paths. It does **not** create a valid empty location and does not skip exposure-list validation. The compiler's Fortran `CHARACTER*200` path bounds apply to compiled paths, and the exposure-list argument is bound by the `strl=512` buffer.

## Mount Object Specification

| Property | Meaning |
| :--- | :--- |
| `host` | Absolute host directory/file; relative strings resolve from the JSON location. |
| `container` | Absolute POSIX destination accessible in the container. |
| `mode` | `ro` for inputs; `rw` when scientific products are written. |
| `kind` | `dir` (default) or `file`. A file mount covers only that file. |
| `create` | Optional; only allowed with a writable directory mount. |

Host mount paths cannot contain unsupported delimiter characters used by the runtime binding format. Protected destinations (system and toolchain directories, the staged source location) cannot be overridden.

### Extra Writable Directories

The run-time dataset tree usually needs write access for stamp, astrometry, and result files. The external catalogs and Gaia tiles can typically remain read-only. If the exposure list is a standalone file outside the processing tree, supply a file mount for it and ensure **all chip paths inside the lists** are also visible.

## Source Isolation and Generated Environment

The F77 launcher creates a **separate source tree** beneath its `run_directory` and writes a generated `f77pipeline.env`. This file records source hashes, image/SIF settings and mounts. Scientific data stay on host storage. Configure again after changing the source, include parameters, JSON paths, or mount layout; then recheck and rebuild as needed.
