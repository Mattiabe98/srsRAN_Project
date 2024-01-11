/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "CLI/CLI11.hpp"

namespace srsran {

struct gnb_parsed_appconfig;

/// Configures the given CLI11 application with the gNB application configuration schema.
void configure_cli11_with_gnb_appconfig_schema(CLI::App& app, gnb_parsed_appconfig& gnb_parsed_cfg);

} // namespace srsran
