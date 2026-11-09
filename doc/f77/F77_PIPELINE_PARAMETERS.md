# 📑 Fortran77 Pipeline Parameter Reference

[Home](../../README.md) · [Essential Configuration](CONFIGURATION.md) · [F77 User Guide](F77_GUIDE.md) · [Data Preparation](DATA_PREPARATION.md)

The scientific Fortran77 executable reads configuration from **compile-time include files**: `f77/para.inc`, `f77/cust_para.inc`, `f77/sig_para.inc`, and `f77/path_layout.inc`. It does not load an INI. The JSON deployment launcher can stage a separate copy of the source and alter **selected path constants** in that staged `para.inc`; numerical scientific options still require source editing and rebuilding.

For new cameras and data sets, review [Essential Configuration](CONFIGURATION.md) before altering numerical thresholds. All table values below describe the currently checked-in F77 source (not another build or fork).

## 1. Basic Numeric Constants and Image Bounds (`para.inc`)

| Constant | Compiled value | Role |
| :--- | :--- | :--- |
| `pi` | `3.1415926` | Mathematical constant |
| `arc_convert` | `pi/180.` | Degree-to-radian scale |
| `npx` | `3000` | Fixed Science-image X buffer limit |
| `npy` | `5000` | Fixed Science-image Y buffer limit |
| `strl` | `512` | Character buffer for list paths |
| `NMAX_CHIP` | Defined in `para.inc` | Compiled chip-array capacity; verify with instrument |
| `NMAX_EXPO` | Defined in `para.inc` | Compiled exposure-list and per-exposure array capacity |

These values influence fixed-form array dimensions and are **not safely adjustable by JSON**. For camera dimensions larger than the compiled bounds, review all callers, memory use, and allocation assumptions before rebuilding.

## 2. Scientific Branch Selectors and Stages (`para.inc`)

| Constant | Compiled value | Interpretation |
| :--- | :--- | :--- |
| `ASTROMETRY_trivial` | `0` | Gaia astrometry rather than identity mapping |
| `PROCESS_stage` | `2*3*5*7*11*13*17*19*23` | Enables all nine numerical stages |
| `include_FLAT` | `0` | Disable flat correction by default |
| `include_Mask` | `2` | DQ-mask mode |
| `PreprocsType` | `1` | Original preprocessing estimator |
| `ext_cat` | `1` | Enable external source-catalog matching |
| `ext_PSF` | `0` | PSF derived from exposure stars |
| `CCD_split` | `2` | Two amplifier halves per physical CCD |
| `deblending` | `1` | Enable source deblending |
| `PSF_type` | `1` | Local polynomial PSF branch |
| `PSF_Ms` | `0` | Multi-scale/PCA PSF reconstruction disabled |

The stage selector tests divisibility by prime factors:

| Factor | Stage | Routine |
| :---: | :---: | :--- |
| `2` | 1 | `pre_process` |
| `3` | 2 | `proc_astrometry` |
| `5` | 3 | `proc_source` |
| `7` | 4 | `proc_FourierT_st1` |
| `11` | 5 | `proc_PSF` |
| `13` | 6 | `proc_FourierT_st2` |
| `17` | 7 | `proc_shear` |
| `19` | 8 | `proc_info` |
| `23` | 9 | `proc_comb` |

Stage skipping is not a generic speed-up: it requires valid intermediate data and compatible prior stage products.

## 3. Catalog and Calibration Paths (`para.inc`)

| Constant | Format / default | Role |
| :--- | :--- | :--- |
| `ASTROMETRY_CAT` | `CHARACTER*200`, original site path | Gaia large-tile directory |
| `SOURCE_CAT` | `CHARACTER*200`, original site path | Canonical external source tile directory |
| `FLAT_PATH` | `CHARACTER*200`, original site path | Super-flat calibration directory |
| `PSF_PATH` | `CHARACTER*200`, placeholder by default | External PSF data root |
| `SOURCE_CAT_TILE_PREFIX` | `extern_` | Prefix for one-degree source tile files |

The four directory paths must match the filesystem visible to the executable, including container mounts. `f77_docker/fqctl.py configure` can substitute those four paths **in the isolated source snapshot**, without altering the original `para.inc`. The `CHARACTER*200` limit applies to every such path.

## 4. PSF Sampling, Detection and Stamp Dimensions (`para.inc`)

| Constant | Value | Purpose |
| :--- | :--- | :--- |
| `psf_order` | `8` | PSF fit order selector |
| `npo`, `npox` | `64`, `8` | Exposure PSF sampling grid |
| `nstar_min` | `npo*3/2` | Minimum exposure PSF star count |
| `npl`, `nplx` | `10`, `2` | Local polynomial specification |
| `nstar_min_local` | `16` | Local PSF fit threshold |
| `step_psf` | `100` | PSF spatial sample step |
| `n_neighbor` | `5` | Deblending neighborhood |
| `nct`, `ncx` | `12`, `3` | Background sampling rectangles |
| `ns`, `nsns` | `64`, `ns*ns` | Stamp side and pixel count |
| `chip_margin` | `8` | Extra extraction border |
| `nl`, `nl_2` | Derived from `ns` and margin | Expanded extraction dimensions |
| `source_thresh` | `2.` | Detection SNR threshold |
| `core_thresh` | `4.` | Source core threshold |
| `flat_thresh` | `0.01` | Flat-field admissibility threshold |
| `flag_thresh` | `3` | Accepted detection flag threshold |
| `area_thresh` | `6` | Source-area threshold |
| `gal_smooth`, `star_smooth` | `2`, `2` | Smoothing selectors |
| `saturation_thresh` | `25000` | Saturation clipping threshold |
| `SNR_PSF` | `100` | PSF candidate SNR selection |
| `pixel_size` | `0.2628` | Camera scale, arcsec/pixel |

These are **scientific** and/or fixed-array settings. They should not be blindly copied to another detector.

## 5. Catalog Buffers and Fourier_Quad Output Columns (`para.inc`)

| Constant | Value | Meaning |
| :--- | :--- | :--- |
| `len_g`, `len_s` | `40`, `15` | Galaxy and star metadata row lengths |
| `ngal_max`, `nstar_max` | `4000`, `2000` | Initial capacity bounds for detections/stars |
| `npara` | `25` | Per-source output parameter count |
| `len_sam` | `50` | PSF sample metadata row length |
| `npd` | `33` | Astrometric distortion parameter count |
| `isig`, `istar`, `i_imax`, `i_jmax` | `4`, `5`, `6`, `7` | Source sigma/star/peak-location fields |
| `ih_flux`, `ih_area`, `iflag` | `8`, `9`, `10` | Photometric area/quality fields |
| `iPSF`, `iSNR_F` | `11`, `12` | PSF size and Fourier SNR fields |
| `ira`, `idec`, `igf1`, `igf2`, `ig1`, `ig2` | Defined contiguously in `para.inc` | Coordinates, distortion, FQ shear estimators |

The numeric column indices are coupled to catalog-generation and downstream readers. Treat them as a **format contract**, not a way to remap a different raw survey schema dynamically.

## 6. Camera and Shear Calibration (`cust_para.inc`, `para.inc`)

| Constant | Value | Purpose |
| :--- | :--- | :--- |
| `chipnx`, `chipny` | `2046`, `4094` | Physical CCD width and height |
| `Camera_ccd_num` | `62` | Expected physical camera CCD count |
| `rescale_size` | `1.2` | Multi-scale PSF reconstruction calibration |
| `procs_pn`, `work_pn` | `40`, `10` | Work/parallelism tuning constants |
| `nblocks` | `2` | Processing block count |
| `n_pcs` | `100` | PCA component buffer count |
| `npp6th` | `28` | Sixth-order coefficient count |
| `nmax_star_pchip` | `1000000` | Upper star capacity per chip |

Other fixed-array and camera-related dimensions appear throughout the scientific `.f` files; review their usage before changing large detector geometries.

## 7. F6 Noise-Plane Estimator (`sig_para.inc`)

| Parameter | Compiled value | Role |
| :--- | :--- | :--- |
| `sig_blocksize`, `sig_block_max` | `200`, `200*200` | Local background block side and pixel capacity |
| `sig_max_blocks` | `2048` | Maximum sampled blocks |
| `sig_min_block_pixels`, `sig_min_block_triples` | `1000`, `1000` | Minimum usable samples |
| `sig_min_blocks` | `4` | Minimum blocks for a plane estimate |
| `sig_hist_nbin`, `sig_hist_range` | `256`, `6.0` | Histogram representation |
| `sig_clip_niter`, `sig_clip_k` | `2`, `3.0` | Iterative clipping |
| `sig_rdil` | `2` | Dilation radius |
| `sig_min_fit_frac` | `0.20` | Minimum fit fraction |
| `sig_median_ratio` | `1.2678405` | Median-to-sigma scale factor |
| `sig_plane_min`, `sig_max_plane_ratio` | `1.e-8`, `4.0` | Plane value bounds |
| `sig_scale_s1`, `sig_scale_s2` | `0.673475`, `1.027786` | Empirical noise calibration |
| `sig_scale` | `sig_scale_s2` | Active estimator scaling |

These values were calibrated for particular noise/statistical assumptions. Do not change them for a new site without an explicit estimator-validation exercise.

## 8. Fixed Relative Product Directories (`path_layout.inc`)

| Representative constant | Directory |
| :--- | :--- |
| `DIR_DQ` | `dqmask` |
| `DIR_NORM` | `stamps/Norm` |
| `DIR_CAT_ORIG` | `stamps/cat_Orig` |
| `DIR_SRC_INFO` | `stamps/dat_SrcInfo` |
| `DIR_SRC`, `DIR_NOISE` | `stamps/fits_Src`, `stamps/fits_Noise` |
| `DIR_PSF_FIT`, `DIR_PSF_LOCAL` | `stamps/dat_PsfFit`, `stamps/fits_PsfLocal` |
| `DIR_SHEAR` | `stamps/dat_Shear` |
| `DIR_ASTRO_DATA`, `DIR_ASTRO_HEAD` | `astrometry/dat_Astro`, `astrometry/Head` |
| `DIR_EXPO_INFO` | `stamps/dat_ExpoInfo` |
| `DIR_PCS` | `stamps/dat_Pcs` |
| `DIR_RESULT` | `result` |

The source file also defines additional stamp, star, PSF and astrometry directories. Preserve the case-sensitive names: consumers derive their paths from these fixed strings.

## 9. Build and Runtime Interface

```bash
make -C f77 clean
make -C f77 LAPACK_LIB_DIR=/path/to/lib \
  CFITSIO_LIB_DIR=/path/to/lib CFITSIO_LIB=/path/to/lib/libcfitsio.so
mpirun -np 4 ./f77/Fourier_Quad_Pipe /absolute/path/to/expo_gband.list
```

The only required application argument is the **existing top-level exposure-list file**. There is no C++-style `--config` parameter in `main.f`. For containerized deployment, the optional JSON launcher stages a copy of the source, writes selected path constants, builds that copy, and passes `paths.exposure_list` as the positional argument.

For scientific reproducibility, archive the exact `.inc` revisions, compiler and container version, exposure list, catalog schemas and processed output set.
