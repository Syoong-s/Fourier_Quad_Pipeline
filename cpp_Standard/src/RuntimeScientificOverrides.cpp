#include "RuntimeScientificOverrides.hpp"
#include "RuntimeConfig.hpp"

#include <algorithm>

namespace RuntimeScientificOverrides {
void apply(const RuntimeConfig& config) {
    const auto& v = config.lensing;
    LensingConfig::PROCESS_stage = v.process_stage;
    LensingConfig::ASTROMETRY_CAT = v.astrometry_cat;
    LensingConfig::CCD_split = v.ccd_split;
    LensingConfig::gal_smooth = v.gal_smooth;
    LensingConfig::star_smooth = v.star_smooth;
    LensingConfig::pixel_size = v.pixel_size;
    LensingConfig::N_CCD = v.nmax_chip;
    LensingConfig::chipnx = v.chipnx;
    LensingConfig::chipny = v.chipny;
    LensingConfig::ASTROMETRY_trivial = v.astrometry_trivial;
    LensingConfig::AstroCatType = v.astrometry_cat_type;
    LensingConfig::include_FLAT = v.include_flat;
    LensingConfig::include_Mask = v.include_mask;
    LensingConfig::FLAT_PATH = v.flat_path;
    LensingConfig::PSF_PATH = v.psf_path;
    LensingConfig::SOURCE_CAT_DEFAULT = config.extcat.output_directory;
    LensingConfig::ext_cat = v.ext_cat;
    LensingConfig::ext_PSF = v.ext_psf;
    LensingConfig::deblending = v.deblending;
    LensingConfig::PSF_type = v.psf_type;
    LensingConfig::PSF_Ms = v.psf_ms;
    ExtCatConfig::EXTCAT_TOTAL_COLUMNS = config.extcat.total_columns;

    // Projected catalogs contain only the selected columns. All downstream
    // column offsets must use the emitted width, not the raw input width.
    const std::size_t emitted_external_columns =
        config.extcat.use_explicit_columns
            ? config.extcat.input_columns_one_based.size()
            : config.extcat.total_columns;

    // Raw external fields are also remapped when the user projects columns.
    const auto mapped = [&](std::size_t raw) -> int {
        if (!config.extcat.use_explicit_columns)
            return static_cast<int>(raw - 1);
        const auto& cols = config.extcat.input_columns_one_based;
        const auto it = std::find(cols.begin(), cols.end(), raw);
        return it == cols.end() ? -1 : static_cast<int>(it - cols.begin());
    };
    FDConfig::col_flags_ft = mapped(1);
    FDConfig::col_flags_fg = mapped(2);
    FDConfig::col_flags_gold = mapped(3);
    FDConfig::col_ext_mash = mapped(4);
    FDConfig::col_cra = mapped(config.extcat.ra_column_one_based);
    FDConfig::col_cdec = mapped(config.extcat.dec_column_one_based);
    FDConfig::col_mag_g = mapped(7);
    FDConfig::col_mag_r = mapped(9);
    FDConfig::col_mag_i = mapped(11);
    FDConfig::col_mag_z = mapped(13);
    FDConfig::col_mag_y = mapped(15);
    FDConfig::col_zp = mapped(config.extcat.zp_column_one_based);
    FDConfig::external_num_cols = static_cast<int>(emitted_external_columns)
        * LensingConfig::ext_cat;
    FDConfig::col_expo = FDConfig::external_num_cols;
    FDConfig::col_ccd = FDConfig::external_num_cols + 1;
    FDConfig::source_col_offset = FDConfig::external_num_cols + 2;
    FDConfig::col_polychi2 = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_polychi2 - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_pixx = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_pixx - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_pixy = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_pixy - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_sig = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_sig - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_star = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_star - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_peak = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_peak - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_imax = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_imax - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_jmax = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_jmax - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_h_flux = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_h_flux - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_h_area = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_h_area - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_flag = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_flag - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_PSF = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_PSF - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_SNR_F = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_SNR_F - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_ra = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_ra - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_dec = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_dec - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_gf1 = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_gf1 - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_gf2 = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_gf2 - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_g1 = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_g1 - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_g2 = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_g2 - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_de = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_de - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_h1 = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_h1 - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_h2 = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_h2 - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_cos2 = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_cos2 - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_sin2 = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_sin2 - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_parity = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_parity - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::col_chi2 = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_col_chi2 - FDConfig::FQ_COMPILED_source_col_offset);
    FDConfig::ICHI2 = FDConfig::source_col_offset + (FDConfig::FQ_COMPILED_ICHI2 - FDConfig::FQ_COMPILED_source_col_offset);
    ProcessRearrConfig::ALL_CAT_TOTAL_COLUMNS = emitted_external_columns
        + ProcessRearrConfig::EXPO_COLUMN_COUNT + ProcessRearrConfig::CCD_COLUMN_COUNT
        + ProcessRearrConfig::ichi2;
}
}  // namespace RuntimeScientificOverrides
