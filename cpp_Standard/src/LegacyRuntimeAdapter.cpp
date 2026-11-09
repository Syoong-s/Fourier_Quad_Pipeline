#include "LegacyRuntimeAdapter.hpp"

#include "ExtCatConfig.hpp"
#include <utility>

namespace LegacyRuntimeAdapter {

bool convert(const RuntimeConfig& runtime,
             ProcessConfig::RuntimeOptions& legacy,
             std::string& error) {
    error.clear();

    // Every field previously available through the application CLI is mapped.
    legacy.workflow.run_astrocat = runtime.process.run_process_astrocat;
    legacy.workflow.run_extcat = runtime.process.run_process_extcat;
    legacy.workflow.run_init = runtime.process.run_process_init;
    legacy.workflow.run_main = runtime.process.run_process_main;
    legacy.workflow.run_rearr = runtime.process.run_process_rearr;
    legacy.workflow.run_fd = runtime.process.run_process_fd;
    legacy.workflow.help_requested = false;

    legacy.pipeline.exposure_list = runtime.process.expo_list;
    legacy.pipeline.external_exposure_list_supplied =
        !runtime.process.expo_list.empty();
    legacy.pipeline.output_root = runtime.init.output_root;
    legacy.pipeline.datasets = runtime.init.datasets;

    legacy.astrocat.input_directory = runtime.astrocat.input_directory;
    legacy.astrocat.output_directory = runtime.astrocat.output_directory;
    legacy.astrocat.add_header = runtime.astrocat.add_header;
    legacy.astrocat.existing_policy = runtime.astrocat.existing_policy;

    // The same output directory supplies process_extcat and process_main.
    legacy.catalog.directory = runtime.extcat.output_directory;
    legacy.catalog.use_explicit_columns = runtime.extcat.use_explicit_columns;
    legacy.catalog.input_columns_one_based =
        runtime.extcat.input_columns_one_based;
    legacy.catalog.use_explicit_coordinate_columns =
        runtime.extcat.use_explicit_coordinate_columns;
    legacy.catalog.ra_column_one_based = runtime.extcat.ra_column_one_based;
    legacy.catalog.dec_column_one_based = runtime.extcat.dec_column_one_based;
    legacy.catalog.zp_column_one_based = runtime.extcat.zp_column_one_based;

    legacy.extcat.input_directory = runtime.extcat.input_directory;
    legacy.extcat.filename_tokens = runtime.extcat.filename_tokens;
    legacy.extcat.recursive = runtime.extcat.recursive;
    legacy.extcat.delimiter = runtime.extcat.delimiter;
    legacy.extcat.header_mode = runtime.extcat.header_mode;
    legacy.extcat.malformed_policy = runtime.extcat.malformed_policy;
    legacy.extcat.existing_policy = runtime.extcat.existing_policy;
    legacy.extcat.chunk_mib = runtime.extcat.chunk_mib;

    legacy.init.science_root = runtime.init.science_root;
    legacy.init.dq_root = runtime.init.dq_root;
    legacy.init.contains = runtime.init.contains;
    legacy.init.existing = runtime.init.existing;
    legacy.init.f77_max_path = runtime.init.f77_max_path;

    legacy.rearr.output_directory = runtime.process.rearr_output_directory;
    legacy.rearr.output_base_directory =
        runtime.process.rearr_output_base_directory;
    legacy.rearr.exposure_list_filename =
        runtime.process.rearranged_expo_list_filename;
    legacy.rearr.exposure_list_directory =
        runtime.process.rearranged_expo_list_directory;

    legacy.fd.exposure_list = runtime.process.fd_expo_list;
    legacy.fd.output_directory = runtime.process.fd_output_directory;
    legacy.fd.output_base_directory = runtime.process.fd_output_base_directory;

    return true;
}
}  // namespace LegacyRuntimeAdapter
