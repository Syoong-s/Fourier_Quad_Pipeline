#include "LegacyRuntimeAdapter.hpp"
#include "RuntimeConfig.hpp"
#include "RuntimeScientificOverrides.hpp"
#include "general/ExposureList.hpp"
#include "general/MPIUtils.hpp"
#include "general/MPIScheduler.hpp"
#include "general/NumericalRecipes.hpp"
#include "general/PathUtils.hpp"
#include "process_astrocat/process_astrocat.hpp"
#include "process_extcat/process_extcat.hpp"
#include "process_fd/process_fd.hpp"
#include "process_init/process_init.hpp"
#include "process_main/ExternalCatalogReader.hpp"
#include "process_main/process_main.hpp"
#include "process_rearr/CatalogRearranger.hpp"
#include "process_rearr/process_rearr.hpp"

#include <mpi.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

int allSucceeded(int local_success) {
    int global_success = 0;
    MPI_Allreduce(&local_success, &global_success, 1, MPI_INT, MPI_MIN,
                  MPIScheduler::state.communicator);
    return global_success;
}

// A single rank reads the input; every rank parses the identical INI text.
bool loadAndBroadcastConfigText(const std::string& path,
                                int rank,
                                std::string& content,
                                std::string& error) {
    int file_ok = 1;
    if (rank == 0) {
        std::ifstream input(path, std::ios::binary);
        if (!input.is_open()) {
            error = "cannot open INI file: " + path;
            file_ok = 0;
        } else {
            std::ostringstream stream;
            stream << input.rdbuf();
            if (!input.good() && !input.eof()) {
                error = "I/O error while reading INI file: " + path;
                file_ok = 0;
            } else {
                content = stream.str();
            }
        }
    }
    MPI_Bcast(&file_ok, 1, MPI_INT, 0, MPIScheduler::state.communicator);
    if (!file_ok) return false;
    return MPIUtils::broadcastString(content, 0, error);
}

void printUsage(const char* program) {
    std::cerr << "Usage: " << program << " [--config PATH]\n"
              << "No arguments: compiled config/*.hpp defaults.\n"
              << "--config PATH / --config=PATH: INI overrides.\n"
              << "No implicit INI search. Other application CLI flags are unsupported.\n";
}

// Preserve the legacy catalog and workflow checks that are more restrictive
// than the generic INI parser's value/type validation.
bool validateLegacyWorkflow(const ProcessConfig::RuntimeOptions& options,
                            std::string& error) {
    if ((options.workflow.run_main || options.workflow.run_extcat)
        && options.catalog.directory.empty()) {
        error = "external source-catalog directory must not be empty";
        return false;
    }
    if (options.workflow.run_main) {
        ExternalCatalogReader::ColumnSelection selection;
        if (!ExternalCatalogReader::resolveColumnSelection(options, selection, error))
            return false;
    }
    if (options.workflow.run_rearr) {
        ProcessRearr::CatalogLayout layout;
        if (!ProcessRearr::resolveCatalogLayout(options, layout, error))
            return false;
    }
    return true;
}

std::string resolveExposureList(const ProcessConfig::RuntimeOptions& options,
                                const InitConfig::DatasetSpec& dataset) {
    std::filesystem::path path;
    if (!options.pipeline.exposure_list.empty()) {
        path = options.pipeline.exposure_list;
    } else {
        if (options.pipeline.output_root.empty()) {
            throw std::runtime_error(
                "expo_list is absent and output_root cannot derive its default");
        }
        path = std::filesystem::path(options.pipeline.output_root)
            / ("expo_" + dataset.target + ".list");
    }
    return PathUtils::normalizedAbsolute(path).string();
}

std::string deriveDatasetRootFromExpoList(const std::string& exposure_list) {
    std::vector<ExposureList::Entry> exposures;
    std::string error;
    if (!ExposureList::loadPipelineList(exposure_list, exposures, 0, error))
        return "";
    for (const auto& exposure : exposures) {
        std::vector<std::string> image_paths;
        if (!ExposureList::loadPathList(exposure.path, image_paths, 0, error)
            || image_paths.empty())
            continue;
        std::filesystem::path dataset_root;
        if (!PathUtils::parentAtLevel(image_paths.front(), 3,
                                      dataset_root, error))
            continue;
        return PathUtils::normalizedAbsolute(dataset_root).string();
    }
    return "";
}

int runWorkflow(const ProcessConfig::RuntimeOptions& options) {
    const int rank = MPIScheduler::state.rank;
    int result = 0;
    if (options.workflow.run_astrocat) {
        if (rank == 0)
            std::cout << "Running process_astrocat before process_extcat\n";
        result = process_astrocat(options);
        if (!result) MPIScheduler::barrier();
    }
    if (!result && options.workflow.run_extcat) {
        if (rank == 0)
            std::cout << "Running process_extcat before dataset phases\n";
        result = process_extcat(options);
        if (!result) MPIScheduler::barrier();
    }

    bool rng_initialized = false;
    const bool needs_dataset = options.workflow.run_init
        || options.workflow.run_main || options.workflow.run_rearr
        || options.workflow.run_fd;
    for (std::size_t i = 0;
         needs_dataset && i < options.pipeline.datasets.size() && !result; ++i) {
        const auto& dataset = options.pipeline.datasets[i];
        if (rank == 0)
            std::cout << "Dataset " << (i+1) << "/"
                      << options.pipeline.datasets.size()
                      << ": target=" << dataset.target
                      << " prefix=" << dataset.prefix << std::endl;

        std::string generated_exposure_list;
        if (options.workflow.run_init)
            result = process_init(options, dataset, generated_exposure_list);

        std::string selected_exposure_list;
        const bool downstream = options.workflow.run_main
            || options.workflow.run_rearr || options.workflow.run_fd;
        if (!result && downstream) {
            if (options.workflow.run_init) {
                selected_exposure_list = generated_exposure_list;
                if (rank == 0)
                    std::cout << "Downstream phases will use process_init output: "
                              << selected_exposure_list << std::endl;
                MPIScheduler::barrier();
            } else {
                int local_ok = 1;
                std::string path_error;
                try {
                    selected_exposure_list = resolveExposureList(options, dataset);
                } catch (const std::exception& exc) {
                    local_ok = 0;
                    path_error = exc.what();
                }
                if (!allSucceeded(local_ok)) {
                    if (rank == 0)
                        std::cerr << "Exposure list error: "
                                  << (path_error.empty() ? "failure on another rank"
                                         : path_error) << std::endl;
                    result = 2;
                }
            }
        }
        if (!result && options.workflow.run_main) {
            if (!rng_initialized) {
                const unsigned int seed = NumericalRecipes::initializeRan1Seed(
                    rank, MPIScheduler::state.size);
                std::cout << "RNG_SEED rank seed: " << rank << " " << seed
                          << std::endl;
                MPIScheduler::barrier();
                rng_initialized = true;
            }
            result = process_main(selected_exposure_list, options);
        }
        if (!result && options.workflow.run_rearr) {
            MPIScheduler::barrier();
            if (rank == 0) std::cout << "Running process_rearr\n";
            result = process_rearr(selected_exposure_list, options);
        }
        if (!result && options.workflow.run_fd) {
            MPIScheduler::barrier();
            if (!rng_initialized) {
                NumericalRecipes::initializeRan1Seed(rank, MPIScheduler::state.size);
                MPIScheduler::barrier();
                rng_initialized = true;
            }
            std::string fd_expo_list;
            if (options.fd.exposure_list.empty()) {
                const std::filesystem::path list_directory =
                    options.rearr.exposure_list_directory.empty()
                        ? std::filesystem::path(selected_exposure_list).parent_path()
                        : std::filesystem::path(options.rearr.exposure_list_directory);
                fd_expo_list = std::filesystem::absolute(
                    list_directory / options.rearr.exposure_list_filename)
                    .lexically_normal().string();
            } else {
                fd_expo_list = options.fd.exposure_list;
            }
            const auto dataset_root = deriveDatasetRootFromExpoList(
                selected_exposure_list);
            if (rank == 0)
                std::cout << "Running process_fd: expo_list=" << fd_expo_list
                          << " dataset_root=" << dataset_root << std::endl;
            result = process_fd(fd_expo_list, options, dataset_root);
        }
        if (!result && i+1 < options.pipeline.datasets.size())
            MPIScheduler::barrier();
    }
    return result;
}
}  // namespace

int main(int argc, char* argv[]) {
    MPIScheduler::init(argc, argv);
    const int rank = MPIScheduler::state.rank;
    if (rank == 0) std::cout << "MPI Init Done..." << std::endl;

    std::string config_path, config_text, error;
    int ok = allSucceeded(findRuntimeConfigPath(
        argc, argv, config_path, error) ? 1 : 0);
    RuntimeConfig runtime_config = makeDefaultRuntimeConfig();
    if (ok && !config_path.empty()) {
        ok = allSucceeded(loadAndBroadcastConfigText(
            config_path, rank, config_text, error) ? 1 : 0);
        if (ok)
            ok = allSucceeded(applyRuntimeConfigText(
                config_text, config_path, runtime_config, error) ? 1 : 0);
    }
    if (ok)
        ok = allSucceeded(validateRuntimeConfig(runtime_config, error) ? 1 : 0);

    ProcessConfig::RuntimeOptions options;
    if (ok)
        ok = allSucceeded(LegacyRuntimeAdapter::convert(
            runtime_config, options, error) ? 1 : 0);
    if (ok) RuntimeScientificOverrides::apply(runtime_config);
    if (ok)
        ok = allSucceeded(validateLegacyWorkflow(options, error) ? 1 : 0);

    if (!ok) {
        if (rank == 0) {
            std::cerr << "Configuration error: "
                      << (error.empty() ? "failure on another MPI rank" : error)
                      << std::endl;
            printUsage(argv[0]);
        }
        MPIScheduler::finalize();
        return 2;
    }

    const int result = runWorkflow(options);
    MPIScheduler::finalize();
    return result;
}
