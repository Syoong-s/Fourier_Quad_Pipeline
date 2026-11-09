#ifndef FQ_RUNTIME_SCIENTIFIC_OVERRIDES_HPP
#define FQ_RUNTIME_SCIENTIFIC_OVERRIDES_HPP

// Preincluded by the Standard Makefile before any configuration header.
// The original config/*.hpp constants remain the single compiled defaults;
// only explicitly INI-configurable fields receive a mutable runtime shadow.
#include <cstddef>
#include <cstdint>
#include <string>

#define PROCESS_stage FQ_COMPILED_PROCESS_stage
#define ASTROMETRY_CAT FQ_COMPILED_ASTROMETRY_CAT
#define CCD_split FQ_COMPILED_CCD_split
#define gal_smooth FQ_COMPILED_gal_smooth
#define star_smooth FQ_COMPILED_star_smooth
#define pixel_size FQ_COMPILED_pixel_size
#define N_CCD FQ_COMPILED_N_CCD
#define chipnx FQ_COMPILED_chipnx
#define chipny FQ_COMPILED_chipny
#define ASTROMETRY_trivial FQ_COMPILED_ASTROMETRY_trivial
#define AstroCatType FQ_COMPILED_AstroCatType
#define include_FLAT FQ_COMPILED_include_FLAT
#define include_Mask FQ_COMPILED_include_Mask
#define FLAT_PATH FQ_COMPILED_FLAT_PATH
#define PSF_PATH FQ_COMPILED_PSF_PATH
#define SOURCE_CAT_DEFAULT FQ_COMPILED_SOURCE_CAT_DEFAULT
#define ext_cat FQ_COMPILED_ext_cat
#define ext_PSF FQ_COMPILED_ext_PSF
#define deblending FQ_COMPILED_deblending
#define PSF_type FQ_COMPILED_PSF_type
#define PSF_Ms FQ_COMPILED_PSF_Ms
#include "../config/LensingConfig.hpp"
#undef PROCESS_stage
#undef ASTROMETRY_CAT
#undef CCD_split
#undef gal_smooth
#undef star_smooth
#undef pixel_size
#undef N_CCD
#undef chipnx
#undef chipny
#undef ASTROMETRY_trivial
#undef AstroCatType
#undef include_FLAT
#undef include_Mask
#undef FLAT_PATH
#undef PSF_PATH
#undef SOURCE_CAT_DEFAULT
#undef ext_cat
#undef ext_PSF
#undef deblending
#undef PSF_type
#undef PSF_Ms

namespace LensingConfig {
inline std::int64_t PROCESS_stage = FQ_COMPILED_PROCESS_stage;
inline std::string ASTROMETRY_CAT = FQ_COMPILED_ASTROMETRY_CAT;
inline int CCD_split = FQ_COMPILED_CCD_split;
inline int gal_smooth = FQ_COMPILED_gal_smooth;
inline int star_smooth = FQ_COMPILED_star_smooth;
inline double pixel_size = FQ_COMPILED_pixel_size;
inline int N_CCD = FQ_COMPILED_N_CCD;
inline int chipnx = FQ_COMPILED_chipnx;
inline int chipny = FQ_COMPILED_chipny;
inline int ASTROMETRY_trivial = FQ_COMPILED_ASTROMETRY_trivial;
inline int AstroCatType = FQ_COMPILED_AstroCatType;
inline int include_FLAT = FQ_COMPILED_include_FLAT;
inline int include_Mask = FQ_COMPILED_include_Mask;
inline std::string FLAT_PATH = FQ_COMPILED_FLAT_PATH;
inline std::string PSF_PATH = FQ_COMPILED_PSF_PATH;
inline std::string SOURCE_CAT_DEFAULT = FQ_COMPILED_SOURCE_CAT_DEFAULT;
inline int ext_cat = FQ_COMPILED_ext_cat;
inline int ext_PSF = FQ_COMPILED_ext_PSF;
inline int deblending = FQ_COMPILED_deblending;
inline int PSF_type = FQ_COMPILED_PSF_type;
inline int PSF_Ms = FQ_COMPILED_PSF_Ms;
}  // namespace LensingConfig

// Keep the baseline catalog-width constant available for constexpr users.
#define EXTCAT_TOTAL_COLUMNS FQ_COMPILED_EXTCAT_TOTAL_COLUMNS
#include "../config/ExtCatConfig.hpp"
#undef EXTCAT_TOTAL_COLUMNS
namespace ExtCatConfig {
inline std::size_t EXTCAT_TOTAL_COLUMNS = FQ_COMPILED_EXTCAT_TOTAL_COLUMNS;
}  // namespace ExtCatConfig

// FDConfig contains compiled catalog-index arithmetic and static_asserts.
// Build that compiled reference with the original constants, then publish
// runtime-adjusted field indices for its numerical consumers.
#define ext_cat FQ_COMPILED_ext_cat
#define EXTCAT_TOTAL_COLUMNS FQ_COMPILED_EXTCAT_TOTAL_COLUMNS
#define col_flags_ft FQ_COMPILED_col_flags_ft
#define col_flags_fg FQ_COMPILED_col_flags_fg
#define col_flags_gold FQ_COMPILED_col_flags_gold
#define col_ext_mash FQ_COMPILED_col_ext_mash
#define col_cra FQ_COMPILED_col_cra
#define col_cdec FQ_COMPILED_col_cdec
#define col_mag_g FQ_COMPILED_col_mag_g
#define col_mag_r FQ_COMPILED_col_mag_r
#define col_mag_i FQ_COMPILED_col_mag_i
#define col_mag_z FQ_COMPILED_col_mag_z
#define col_mag_y FQ_COMPILED_col_mag_y
#define col_zp FQ_COMPILED_col_zp
#define external_num_cols FQ_COMPILED_external_num_cols
#define col_expo FQ_COMPILED_col_expo
#define col_ccd FQ_COMPILED_col_ccd
#define source_col_offset FQ_COMPILED_source_col_offset
#define col_polychi2 FQ_COMPILED_col_polychi2
#define col_pixx FQ_COMPILED_col_pixx
#define col_pixy FQ_COMPILED_col_pixy
#define col_sig FQ_COMPILED_col_sig
#define col_star FQ_COMPILED_col_star
#define col_peak FQ_COMPILED_col_peak
#define col_imax FQ_COMPILED_col_imax
#define col_jmax FQ_COMPILED_col_jmax
#define col_h_flux FQ_COMPILED_col_h_flux
#define col_h_area FQ_COMPILED_col_h_area
#define col_flag FQ_COMPILED_col_flag
#define col_PSF FQ_COMPILED_col_PSF
#define col_SNR_F FQ_COMPILED_col_SNR_F
#define col_ra FQ_COMPILED_col_ra
#define col_dec FQ_COMPILED_col_dec
#define col_gf1 FQ_COMPILED_col_gf1
#define col_gf2 FQ_COMPILED_col_gf2
#define col_g1 FQ_COMPILED_col_g1
#define col_g2 FQ_COMPILED_col_g2
#define col_de FQ_COMPILED_col_de
#define col_h1 FQ_COMPILED_col_h1
#define col_h2 FQ_COMPILED_col_h2
#define col_cos2 FQ_COMPILED_col_cos2
#define col_sin2 FQ_COMPILED_col_sin2
#define col_parity FQ_COMPILED_col_parity
#define col_chi2 FQ_COMPILED_col_chi2
#define ICHI2 FQ_COMPILED_ICHI2
#include "../config/FDConfig.hpp"
#undef ext_cat
#undef EXTCAT_TOTAL_COLUMNS
#undef col_flags_ft
#undef col_flags_fg
#undef col_flags_gold
#undef col_ext_mash
#undef col_cra
#undef col_cdec
#undef col_mag_g
#undef col_mag_r
#undef col_mag_i
#undef col_mag_z
#undef col_mag_y
#undef col_zp
#undef external_num_cols
#undef col_expo
#undef col_ccd
#undef source_col_offset
#undef col_polychi2
#undef col_pixx
#undef col_pixy
#undef col_sig
#undef col_star
#undef col_peak
#undef col_imax
#undef col_jmax
#undef col_h_flux
#undef col_h_area
#undef col_flag
#undef col_PSF
#undef col_SNR_F
#undef col_ra
#undef col_dec
#undef col_gf1
#undef col_gf2
#undef col_g1
#undef col_g2
#undef col_de
#undef col_h1
#undef col_h2
#undef col_cos2
#undef col_sin2
#undef col_parity
#undef col_chi2
#undef ICHI2

namespace FDConfig {
inline int col_flags_ft = FQ_COMPILED_col_flags_ft;
inline int col_flags_fg = FQ_COMPILED_col_flags_fg;
inline int col_flags_gold = FQ_COMPILED_col_flags_gold;
inline int col_ext_mash = FQ_COMPILED_col_ext_mash;
inline int col_cra = FQ_COMPILED_col_cra;
inline int col_cdec = FQ_COMPILED_col_cdec;
inline int col_mag_g = FQ_COMPILED_col_mag_g;
inline int col_mag_r = FQ_COMPILED_col_mag_r;
inline int col_mag_i = FQ_COMPILED_col_mag_i;
inline int col_mag_z = FQ_COMPILED_col_mag_z;
inline int col_mag_y = FQ_COMPILED_col_mag_y;
inline int col_zp = FQ_COMPILED_col_zp;
inline int external_num_cols = static_cast<int>(ExtCatConfig::EXTCAT_TOTAL_COLUMNS) * LensingConfig::ext_cat;
inline int col_expo = external_num_cols;
inline int col_ccd = external_num_cols + 1;
inline int source_col_offset = external_num_cols + 2;
inline int col_polychi2 = source_col_offset + (FQ_COMPILED_col_polychi2 - FQ_COMPILED_source_col_offset);
inline int col_pixx = source_col_offset + (FQ_COMPILED_col_pixx - FQ_COMPILED_source_col_offset);
inline int col_pixy = source_col_offset + (FQ_COMPILED_col_pixy - FQ_COMPILED_source_col_offset);
inline int col_sig = source_col_offset + (FQ_COMPILED_col_sig - FQ_COMPILED_source_col_offset);
inline int col_star = source_col_offset + (FQ_COMPILED_col_star - FQ_COMPILED_source_col_offset);
inline int col_peak = source_col_offset + (FQ_COMPILED_col_peak - FQ_COMPILED_source_col_offset);
inline int col_imax = source_col_offset + (FQ_COMPILED_col_imax - FQ_COMPILED_source_col_offset);
inline int col_jmax = source_col_offset + (FQ_COMPILED_col_jmax - FQ_COMPILED_source_col_offset);
inline int col_h_flux = source_col_offset + (FQ_COMPILED_col_h_flux - FQ_COMPILED_source_col_offset);
inline int col_h_area = source_col_offset + (FQ_COMPILED_col_h_area - FQ_COMPILED_source_col_offset);
inline int col_flag = source_col_offset + (FQ_COMPILED_col_flag - FQ_COMPILED_source_col_offset);
inline int col_PSF = source_col_offset + (FQ_COMPILED_col_PSF - FQ_COMPILED_source_col_offset);
inline int col_SNR_F = source_col_offset + (FQ_COMPILED_col_SNR_F - FQ_COMPILED_source_col_offset);
inline int col_ra = source_col_offset + (FQ_COMPILED_col_ra - FQ_COMPILED_source_col_offset);
inline int col_dec = source_col_offset + (FQ_COMPILED_col_dec - FQ_COMPILED_source_col_offset);
inline int col_gf1 = source_col_offset + (FQ_COMPILED_col_gf1 - FQ_COMPILED_source_col_offset);
inline int col_gf2 = source_col_offset + (FQ_COMPILED_col_gf2 - FQ_COMPILED_source_col_offset);
inline int col_g1 = source_col_offset + (FQ_COMPILED_col_g1 - FQ_COMPILED_source_col_offset);
inline int col_g2 = source_col_offset + (FQ_COMPILED_col_g2 - FQ_COMPILED_source_col_offset);
inline int col_de = source_col_offset + (FQ_COMPILED_col_de - FQ_COMPILED_source_col_offset);
inline int col_h1 = source_col_offset + (FQ_COMPILED_col_h1 - FQ_COMPILED_source_col_offset);
inline int col_h2 = source_col_offset + (FQ_COMPILED_col_h2 - FQ_COMPILED_source_col_offset);
inline int col_cos2 = source_col_offset + (FQ_COMPILED_col_cos2 - FQ_COMPILED_source_col_offset);
inline int col_sin2 = source_col_offset + (FQ_COMPILED_col_sin2 - FQ_COMPILED_source_col_offset);
inline int col_parity = source_col_offset + (FQ_COMPILED_col_parity - FQ_COMPILED_source_col_offset);
inline int col_chi2 = source_col_offset + (FQ_COMPILED_col_chi2 - FQ_COMPILED_source_col_offset);
inline int ICHI2 = source_col_offset + (FQ_COMPILED_ICHI2 - FQ_COMPILED_source_col_offset);
}  // namespace FDConfig

// ProcessRearrConfig has one constexpr default width plus a runtime width
// helper; keep the old definition for static assertions and rebind both.
#define EXTCAT_TOTAL_COLUMNS FQ_COMPILED_EXTCAT_TOTAL_COLUMNS
#define ALL_CAT_TOTAL_COLUMNS FQ_COMPILED_ALL_CAT_TOTAL_COLUMNS
#define externalCatalogColumns FQ_COMPILED_externalCatalogColumns
#define allCatalogColumns FQ_COMPILED_allCatalogColumns
#include "../config/ProcessRearrConfig.hpp"
#undef EXTCAT_TOTAL_COLUMNS
#undef ALL_CAT_TOTAL_COLUMNS
#undef externalCatalogColumns
#undef allCatalogColumns

namespace ProcessRearrConfig {
inline std::size_t ALL_CAT_TOTAL_COLUMNS = ExtCatConfig::EXTCAT_TOTAL_COLUMNS
    + EXPO_COLUMN_COUNT + CCD_COLUMN_COUNT + ichi2;
inline std::size_t externalCatalogColumns(const ProcessConfig::RuntimeOptions& options) {
    return options.catalog.use_explicit_columns
        ? options.catalog.input_columns_one_based.size()
        : ExtCatConfig::EXTCAT_TOTAL_COLUMNS;
}
inline std::size_t allCatalogColumns(const ProcessConfig::RuntimeOptions& options) {
    return externalCatalogColumns(options) + EXPO_COLUMN_COUNT + CCD_COLUMN_COUNT + ichi2;
}
}  // namespace ProcessRearrConfig

struct RuntimeConfig;
namespace RuntimeScientificOverrides {
// Call on every MPI rank once the INI and runtime options are validated.
void apply(const RuntimeConfig& config);
}

#endif  // FQ_RUNTIME_SCIENTIFIC_OVERRIDES_HPP
