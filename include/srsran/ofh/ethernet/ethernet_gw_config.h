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

#include "srsran/ofh/ethernet/ethernet_mac_address.h"
#include "srsran/support/units.h"

namespace srsran {
namespace ether {

/// Configuration for the Ethernet gateway.
struct gw_config {
  /// Interface name.
  std::string interface;
  /// Promiscuous mode flag.
  bool is_promiscuous_mode_enabled;
  /// MTU size.
  units::bytes mtu_size;
  /// Destination MAC address.
  mac_address mac_dst_address;
};

} // namespace ether
} // namespace srsran
