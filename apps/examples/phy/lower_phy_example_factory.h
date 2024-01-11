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

#include "srsran/phy/lower/lower_phy.h"
#include "srsran/phy/lower/lower_phy_configuration.h"
#include <memory>

namespace srsran {

/// Creates a generic lower PHY.
std::unique_ptr<lower_phy> create_lower_phy(lower_phy_configuration& config);

} // namespace srsran
