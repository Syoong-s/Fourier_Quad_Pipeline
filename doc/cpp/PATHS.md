# 🔗 C++ Container Path Reference

[Home](../../README.md) · [Docker](DOCKER.md) · [HPC](HPC.md) · [Parameter Reference](CPP_PIPELINE_PARAMETERS.md)

The C++ executable only sees paths inside its current runtime namespace. In Docker or Apptainer, `cpp_docker/project.json` translates host storage into container destinations and then into generated INI values. Native runs do not use `project.json`; write native paths directly into your chosen `pipeline.ini`.

## How `mounts` and `paths` Connect

- `mounts` binds host directories or individual files into the container.
- `paths` assigns the **container destination** to a recognized `[section].key` in the generated INI.
- A path must be covered by a mount; the launcher uses the **longest matching parent mount**.
- For output paths, use writable mounts with `"mode": "rw"`; an allowed new writable directory can use `"create": true`.

```json
{
  "mounts": [
    {"host": "/observations", "container": "/data/archive", "mode": "ro"},
    {"host": "/reference", "container": "/data/catalogs", "mode": "ro"},
    {"host": "/analysis", "container": "/data/output", "mode": "rw"}
  ],
  "paths": {
    "init.science_root": "/data/archive/science",
    "init.dq_root": "/data/archive/dq",
    "init.output_root": "/data/output",
    "lensing.astrometry_cat": "/data/catalogs/gaia",
    "lensing.source_cat": "/data/catalogs/external"
  }
}
```

The excerpt is part of the full JSON project; see [Docker](DOCKER.md) for other required fields. The program uses **`/data/archive/science`**, not the host directory `/observations/science`.

### Mount object specification

| Property | Type | Meaning |
| :--- | :--- | :--- |
| `host` | Host path | Existing input directory/file, or a permitted new writable output directory. Relative paths are resolved from the JSON's directory. |
| `container` | Absolute POSIX path | Destination used by generated INI and all MPI ranks. |
| `mode` | `ro` / `rw` | Read-only input versus writable product destination. |
| `create` | Boolean | Create the new host output directory during `configure`; requires a writable directory mount. |

Reserved destinations such as `/usr`, `/etc`, `/workspace/src_pipe`, and `/fq-config` cannot be overridden. Mounting a single file differs from mounting a directory: use the exact file path, not arbitrary children of that file.

## Complete `paths` Key Reference

Every key below maps to an **existing supported C++ INI key**. Null or omitted JSON values inherit compiled defaults except where the launcher explicitly links a producer/consumer path. For active empty strings, review the INI setting's empty-value semantics.

| JSON `paths` key | C++ INI destination | Usage |
| :--- | :--- | :--- |
| `init.science_root` | `[init] science_root` | Science archive root |
| `init.dq_root` | `[init] dq_root` | DQ archive root |
| `init.output_root` | `[init] output_root` | Writable dataset/output root |
| `astrocat.input_directory` | `[astrocat] input_directory` | Raw Gaia catalog directory |
| `astrocat.output_directory` | `[astrocat] output_directory` | Generated Gaia tile directory |
| `extcat.input_directory` | `[extcat] input_directory` | Raw source catalog directory |
| `extcat.output_directory` | `[extcat] output_directory` | Produced/consumed source tile directory |
| `lensing.astrometry_cat` | `[lensing] astrometry_cat` | Main-phase Gaia tile directory |
| `lensing.source_cat` | `[lensing] source_cat` | Alias for the effective external tile directory |
| `lensing.flat_path` | `[lensing] flat_path` | Flat calibration files |
| `lensing.psf_path` | `[lensing] psf_path` | External PSF files |
| `process.expo_list` | `[process] expo_list` | Existing top-level exposure list |
| `process.rearr_output_directory` | `[process] rearr_output_directory` | Rearranged directory name/location |
| `process.rearr_output_base_directory` | `[process] rearr_output_base_directory` | Rearrangement base path |
| `process.rearranged_expo_list_filename` | `[process] rearranged_expo_list_filename` | Name of generated rearranged list |
| `process.rearranged_expo_list_directory` | `[process] rearranged_expo_list_directory` | Directory holding rearranged list |
| `process.fd_expo_list` | `[process] fd_expo_list` | Field-distortion exposure list |
| `process.fd_output_directory` | `[process] fd_output_directory` | FD output directory name/location |
| `process.fd_output_base_directory` | `[process] fd_output_base_directory` | FD output base path |

### Linked producer & consumer paths

The C++ runtime ties `[extcat].output_directory` and `[lensing].source_cat` to a single effective external catalog directory. The launcher resolves their alias before writing INI. **Do not set contradictory destinations.** Gaia producer `[astrocat].output_directory` and main consumer `[lensing].astrometry_cat` are independent; set both when consuming newly generated one-degree Gaia tiles.

## Exposure Lists and Additional Files

For downstream-only runs, the referenced top-level exposure-list file must be accessible inside the container; **all chip paths stored inside the list hierarchy** must also be valid container paths. A file mount covering the list itself does not automatically remap its contents.

For specialized calibration inputs or non-default stage outputs, add new `mounts` first and then set the corresponding known `paths` key. Do not add arbitrary unsupported JSON keys: the launcher validates its schema and will reject unknown fields.
