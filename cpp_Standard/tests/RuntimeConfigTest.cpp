#include "LegacyRuntimeAdapter.hpp"
#include "RuntimeConfig.hpp"
#include "RuntimeScientificOverrides.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace {
int failed = 0;
int assertions = 0;
void expect(bool condition, const std::string& message) {
    ++assertions;
    if (!condition) { ++failed; std::cerr << "FAIL: " << message << '\n'; }
}

bool scan(std::vector<std::string> args, std::string& path) {
    std::vector<char*> argv;
    for (auto& a : args) argv.push_back(&a[0]);
    std::string error;
    return findRuntimeConfigPath(static_cast<int>(argv.size()), argv.data(),
                                 path, error);
}
}

int main() {
    std::string path, error;
    expect(scan({"fq"}, path) && path.empty(),
           "no arguments use compiled defaults");
    expect(scan({"fq", "--config", "some file.ini"}, path)
           && path == "some file.ini", "--config PATH");
    expect(scan({"fq", "--config=test.ini"}, path) && path == "test.ini",
           "--config=PATH");
    for (const auto& args : std::vector<std::vector<std::string>>{
             {"fq", "--help"}, {"fq", "--dataset", "target:prefix"},
             {"fq", "old.list"}, {"fq", "--config"},
             {"fq", "--config="},
             {"fq", "--config", "one", "--config", "two"},
             {"fq", "--config", "a.ini", "unexpected"}})
        expect(!scan(args, path), "reject unsupported/malformed command line");

    const RuntimeConfig compiled = makeDefaultRuntimeConfig();
    ProcessConfig::RuntimeOptions legacy;
    expect(LegacyRuntimeAdapter::convert(compiled, legacy, error),
           "compiled configuration must be accepted by legacy adapter: " + error);
    expect(legacy.workflow.run_init == ProcessConfig::RUN_PROCESS_INIT,
           "compiled init phase default retained");
    expect(legacy.workflow.run_fd == ProcessConfig::RUN_PROCESS_FD,
           "compiled FD phase default retained");
    expect(legacy.catalog.directory == ExtCatConfig::EXTCAT_OUTPUT_DIRECTORY,
           "compiled catalog path retained");
    expect(legacy.pipeline.datasets.size() == InitConfig::DATASETS.size(),
           "compiled dataset default retained");

    RuntimeConfig config = compiled;
    const std::string ini =
        "[process]\n"
        "run_process_init = false\n"
        "run_process_main = true\n"
        "run_process_rearr = false\n"
        "run_process_fd = false\n"
        "expo_list = \"/data/my list.txt\"\n"
        "[init]\n"
        "science_root = /new/science\n"
        "output_root = /new/output\n"
        "datasets = gband:c4d_\n"
        "contains =\n"
        "[lensing]\n"
        "source_cat = /new/source_tiles\n"
        "[extcat]\n"
        "ra_column = 3\n"
        "dec_column = 4\n"
        "zp_column = 7\n"
        "use_explicit_coordinate_columns = true\n"
        "[astrocat]\n"
        "input_directory = /data/gaia_raw\n";
    expect(applyRuntimeConfigText(ini, "test.ini", config, error),
           "apply workflow, dataset, and catalog overrides: " + error);
    expect(validateRuntimeConfig(config, error),
           "validate partial override: " + error);
    expect(LegacyRuntimeAdapter::convert(config, legacy, error),
           "convert partial override: " + error);
    expect(!legacy.workflow.run_init && legacy.workflow.run_main,
           "INI replaces selected phase defaults");
    expect(legacy.pipeline.exposure_list == "/data/my list.txt",
           "quoted filename retained");
    expect(legacy.pipeline.datasets.size() == 1 && legacy.init.contains.empty(),
           "datasets replaced and filter cleared");
    expect(legacy.catalog.directory == "/new/source_tiles"
           && legacy.catalog.ra_column_one_based == 3
           && legacy.catalog.dec_column_one_based == 4,
           "INI source alias and coordinate fields applied");
    expect(legacy.astrocat.input_directory == "/data/gaia_raw",
           "Gaia preparation path applied");
    expect(config.lensing.pixel_size == compiled.lensing.pixel_size,
           "unmentioned science value retains compiled default");

    for (const auto& bad : std::vector<std::string>{
             "[process]\nrun_process_fd = invalid\n",
             "[init]\nnot_a_real_key = 1\n",
             "[lensing]\npixel_size =\n",
             "[missing]\nanything = 1\n"}) {
        RuntimeConfig candidate = compiled;
        expect(!applyRuntimeConfigText(bad, "invalid.ini", candidate, error),
               "invalid INI must fail");
        expect(candidate.init.datasets.size() == compiled.init.datasets.size(),
               "failed INI must be transactional");
    }
    config = compiled;
    const std::string full_workflow_ini =
        "[process]\n"
        "rearr_output_directory = sorted\n"
        "rearr_output_base_directory = /output/rearr\n"
        "rearranged_expo_list_filename = ready.list\n"
        "rearranged_expo_list_directory = /output/lists\n"
        "fd_expo_list = /output/fd_input.list\n"
        "fd_output_directory = fd-products\n"
        "fd_output_base_directory = /output/fd\n"
        "[astrocat]\n"
        "add_header = false\n"
        "existing_policy = overwrite\n"
        "[extcat]\n"
        "input_directory = /raw/source\n"
        "filename_tokens = red, blue\n"
        "recursive = false\n"
        "delimiter = comma\n"
        "header_mode = present\n"
        "malformed_policy = skip\n"
        "existing_policy = overwrite\n"
        "chunk_mib = 128\n"
        "use_explicit_columns = true\n"
        "input_columns = 3, 4, 7\n"
        "use_explicit_coordinate_columns = true\n"
        "ra_column = 3\n"
        "dec_column = 4\n"
        "zp_column = 7\n"
        "[init]\n"
        "existing = resume\n"
        "f77_max_path = 255\n";
    expect(applyRuntimeConfigText(full_workflow_ini, "complete.ini", config, error),
           "parse all legacy CLI fields from INI: " + error);
    expect(validateRuntimeConfig(config, error),
           "validate all legacy CLI fields: " + error);
    expect(LegacyRuntimeAdapter::convert(config, legacy, error),
           "convert all legacy CLI fields: " + error);
    expect(legacy.rearr.output_directory == "sorted"
           && legacy.rearr.output_base_directory == "/output/rearr"
           && legacy.rearr.exposure_list_filename == "ready.list"
           && legacy.rearr.exposure_list_directory == "/output/lists",
           "rearrangement CLI fields mapped");
    expect(legacy.fd.exposure_list == "/output/fd_input.list"
           && legacy.fd.output_directory == "fd-products"
           && legacy.fd.output_base_directory == "/output/fd",
           "FD CLI fields mapped");
    expect(!legacy.astrocat.add_header
           && legacy.astrocat.existing_policy == "overwrite",
           "Gaia CLI fields mapped");
    expect(legacy.extcat.input_directory == "/raw/source"
           && legacy.extcat.filename_tokens.size() == 2
           && !legacy.extcat.recursive
           && legacy.extcat.delimiter == "comma"
           && legacy.extcat.header_mode == "present"
           && legacy.extcat.malformed_policy == "skip"
           && legacy.extcat.existing_policy == "overwrite"
           && legacy.extcat.chunk_mib == 128,
           "external catalog CLI fields mapped");
    expect(legacy.catalog.use_explicit_columns
           && legacy.catalog.input_columns_one_based.size() == 3
           && legacy.catalog.input_columns_one_based.front() == 3
           && legacy.catalog.use_explicit_coordinate_columns,
           "ordered projection and explicit coordinate controls mapped");
    expect(legacy.init.existing == "resume"
           && legacy.init.f77_max_path == 255,
           "initializer CLI fields mapped");

    config = compiled;
    expect(applyRuntimeConfigText("[lensing]\npixel_size = 0.35\n",
                                  "science.ini", config, error),
           "syntax of scientific key understood");
    expect(LegacyRuntimeAdapter::convert(config, legacy, error),
           "valid scientific override must be accepted by adapter");
    RuntimeScientificOverrides::apply(config);
    expect(LensingConfig::pixel_size == 0.35,
           "pixel_size override reaches the actual science namespace");
    config = compiled;
    expect(applyRuntimeConfigText("[extcat]\ntotal_columns = 99\n",
                                  "legacy.ini", config, error),
           "parse extcat width");
    expect(LegacyRuntimeAdapter::convert(config, legacy, error),
           "valid catalog-width override must be accepted");
    RuntimeScientificOverrides::apply(config);
    expect(ExtCatConfig::EXTCAT_TOTAL_COLUMNS == 99
           && ProcessRearrConfig::externalCatalogColumns(legacy) == 99
           && FDConfig::col_expo == 99,
           "catalog width reaches rearrangement and FD indices");

    // Exercise every INI-configurable science value against the variables
    // actually read by the unchanged legacy scientific implementation.
    config = compiled;
    expect(applyRuntimeConfigText(
        "[lensing]\n"
        "process_stage = 30\n"
        "astrometry_cat = /other/gaia\n"
        "ccd_split = 1\n"
        "gal_smooth = 3\n"
        "star_smooth = 4\n"
        "pixel_size = 0.35\n"
        "nmax_chip = 16\n"
        "chipnx = 4096\n"
        "chipny = 2048\n"
        "astrometry_trivial = 1\n"
        "astrometry_cat_type = 2\n"
        "include_flat = 1\n"
        "include_mask = 3\n"
        "flat_path = /other/flat\n"
        "psf_path = /other/psf\n"
        "ext_cat = 0\n"
        "ext_psf = 1\n"
        "deblending = 0\n"
        "psf_type = 2\n"
        "psf_ms = 1\n"
        "source_cat = /other/source\n",
        "all-science.ini", config, error), "parse all science overrides: " + error);
    expect(validateRuntimeConfig(config, error), "validate all science overrides: " + error);
    RuntimeScientificOverrides::apply(config);
#define CHECK_SCIENCE(alias, value) expect(LensingConfig::alias == (value), \
        "science consumer must see overridden " #alias)
    CHECK_SCIENCE(PROCESS_stage, 30);
    CHECK_SCIENCE(ASTROMETRY_CAT, "/other/gaia");
    CHECK_SCIENCE(CCD_split, 1);
    CHECK_SCIENCE(gal_smooth, 3);
    CHECK_SCIENCE(star_smooth, 4);
    CHECK_SCIENCE(pixel_size, 0.35);
    CHECK_SCIENCE(N_CCD, 16);
    CHECK_SCIENCE(chipnx, 4096);
    CHECK_SCIENCE(chipny, 2048);
    CHECK_SCIENCE(ASTROMETRY_trivial, 1);
    CHECK_SCIENCE(AstroCatType, 2);
    CHECK_SCIENCE(include_FLAT, 1);
    CHECK_SCIENCE(include_Mask, 3);
    CHECK_SCIENCE(FLAT_PATH, "/other/flat");
    CHECK_SCIENCE(PSF_PATH, "/other/psf");
    CHECK_SCIENCE(SOURCE_CAT_DEFAULT, "/other/source");
    CHECK_SCIENCE(ext_cat, 0);
    CHECK_SCIENCE(ext_PSF, 1);
    CHECK_SCIENCE(deblending, 0);
    CHECK_SCIENCE(PSF_type, 2);
    CHECK_SCIENCE(PSF_Ms, 1);
#undef CHECK_SCIENCE
    expect(FDConfig::external_num_cols == 0 && FDConfig::col_expo == 0,
           "FD indices follow ext_cat=0");
    expect(makeDefaultRuntimeConfig().lensing.pixel_size == compiled.lensing.pixel_size
           && makeDefaultRuntimeConfig().lensing.nmax_chip == compiled.lensing.nmax_chip,
           "compiled factory defaults stay stable after runtime overrides");

    config = compiled;
    expect(applyRuntimeConfigText(
        "[process]\nrun_process_fd = false\n"
        "[extcat]\ntotal_columns = 21\n"
        "ra_column = 3\ndec_column = 4\nzp_column = 8\n"
        "use_explicit_columns = true\n"
        "input_columns = 8, 4, 3, 7, 9, 11, 13, 15\n",
        "projection.ini", config, error), "parse projected catalog fields: " + error);
    expect(validateRuntimeConfig(config, error),
           "projected catalog allowed when FD is disabled: " + error);
    expect(LegacyRuntimeAdapter::convert(config, legacy, error),
           "projection reaches legacy reader: " + error);
    RuntimeScientificOverrides::apply(config);
    expect(ExtCatConfig::EXTCAT_TOTAL_COLUMNS == 21,
           "compiled catalog width reference is overridden");
    expect(ProcessRearrConfig::externalCatalogColumns(legacy) == 8
           && ProcessRearrConfig::allCatalogColumns(legacy) == 35,
           "rearrangement resolves projected catalog width");
    expect(FDConfig::col_cra == 2 && FDConfig::col_cdec == 1
           && FDConfig::col_zp == 0,
           "FD coordinates map raw one-based columns through projection");
    config.process.run_process_fd = true;
    expect(!validateRuntimeConfig(config, error)
           && error.find("process_fd") != std::string::npos,
           "FD requires every legacy fixed external source field in projection");

    std::cout << assertions << " assertions, " << failed << " failures\n";
    return failed ? 1 : 0;
}
