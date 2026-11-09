#ifndef LEGACY_RUNTIME_ADAPTER_HPP
#define LEGACY_RUNTIME_ADAPTER_HPP

#include "ProcessConfig.hpp"
#include "RuntimeConfig.hpp"
#include <string>

// Translate the INI model into the existing scientific workflow's option
// structures without changing the legacy stage implementations.
namespace LegacyRuntimeAdapter {
bool convert(const RuntimeConfig& runtime,
             ProcessConfig::RuntimeOptions& legacy,
             std::string& error);
}

#endif
