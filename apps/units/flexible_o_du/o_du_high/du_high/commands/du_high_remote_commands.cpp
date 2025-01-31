/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "apps/units/flexible_o_du/o_du_high/du_high/commands/du_high_remote_commands.h"
#include "nlohmann/json.hpp"

using namespace srsran;

error_type<std::string> ssb_modify_remote_command::execute(const nlohmann::json& json)
{
  auto cells_key = json.find("cells");
  if (cells_key == json.end()) {
    return make_unexpected("'cells' object is missing and it is mandatory");
  }
  if (!cells_key->is_array()) {
    return make_unexpected("'cells' object vale type should be an array");
  }

  auto cells_items = cells_key->items();
  if (cells_items.begin() == cells_items.end()) {
    return make_unexpected("'cells' object does not contain any cell entries");
  }

  srs_du::du_param_config_request req;
  for (const auto& cell : cells_items) {
    auto plmn_key = cell.value().find("plmn");
    if (plmn_key == cell.value().end()) {
      return make_unexpected("'plmn' object is missing and it is mandatory");
    }
    if (!plmn_key->is_string()) {
      return make_unexpected("'plmn' object vale type should be a string");
    }

    auto nci_key = cell.value().find("nci");
    if (nci_key == cell.value().end()) {
      return make_unexpected("'nci' object is missing and it is mandatory");
    }
    if (!nci_key->is_number_unsigned()) {
      return make_unexpected("'nci' object vale type should be an integer");
    }

    auto plmn = plmn_identity::parse(plmn_key.value().get_ref<const nlohmann::json::string_t&>());
    if (!plmn) {
      return make_unexpected("Invalid PLMN identity value");
    }
    auto nci = nr_cell_identity::create(nci_key->get<uint64_t>());
    if (!nci) {
      return make_unexpected("Invalid NR cell identity value");
    }
    nr_cell_global_id_t nr_cgi;
    nr_cgi.nci     = nci.value();
    nr_cgi.plmn_id = plmn.value();

    auto ssb_block_power_key = cell.value().find("ssb_block_power");
    if (ssb_block_power_key == cell.value().end()) {
      return make_unexpected("'ssb_block_power' object is missing and it is mandatory");
    }
    if (!ssb_block_power_key->is_number_integer()) {
      return make_unexpected("'ssb_block_power' object vale type should be an integer");
    }
    int ssb_block_power_value = ssb_block_power_key->get<int>();
    if (ssb_block_power_value < -60 || ssb_block_power_value > 50) {
      return make_unexpected(fmt::format(
          "'ssb_block_power' value out of range, received '{}', valid range is from -60 to 50", ssb_block_power_value));
    }
    req.cells.emplace_back(nr_cgi, ssb_block_power_value);
  }

  if (configurator.handle_operator_config_request(req).success) {
    return {};
  }

  return make_unexpected("SSB modify command could not be applied by the DU");
}
