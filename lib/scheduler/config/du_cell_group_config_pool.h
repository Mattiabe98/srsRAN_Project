/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "logical_channel_config_pool.h"
#include "sched_config_params.h"
#include "srsran/ran/du_types.h"
#include "srsran/scheduler/config/bwp_configuration.h"
#include "srsran/scheduler/config/serving_cell_config.h"
#include <map>

namespace srsran {

struct sched_cell_configuration_request_message;
struct sched_ue_creation_request_message;
struct sched_ue_reconfiguration_message;
struct cell_config_dedicated;

class du_cell_config_pool
{
public:
  du_cell_config_pool(const sched_cell_configuration_request_message& cell_cfg);
  du_cell_config_pool(const du_cell_config_pool&)            = delete;
  du_cell_config_pool& operator=(const du_cell_config_pool&) = delete;

  ue_cell_config_ptr update_ue(const serving_cell_config& ue_cell);

private:
  void add_bwp(ue_cell_config&               out,
               bwp_id_t                      bwp_id,
               const bwp_downlink_common&    dl_bwp_common,
               const bwp_downlink_dedicated* dl_bwp_ded,
               const bwp_uplink_common*      ul_bwp_common,
               const bwp_uplink_dedicated*   ul_bwp_ded);

  const bwp_downlink_common init_dl_bwp;
  const bwp_uplink_common   init_ul_bwp;

  config_object_pool<ue_cell_config>            cell_cfg_pool;
  config_object_pool<serving_cell_config>       serving_cell_cfg_pool;
  config_object_pool<bwp_config>                bwp_config_pool;
  config_object_pool<coreset_configuration>     coreset_config_pool;
  config_object_pool<uplink_config>             ul_cfg_pool;
  config_object_pool<pdsch_serving_cell_config> pdsch_serv_cell_pool;
  config_object_pool<csi_meas_config>           csi_meas_config_pool;
};

/// Class responsible for managing the configurations of all the entities (cells, UEs, slices) in a cell group.
class du_cell_group_config_pool
{
public:
  /// Creates handles to the resources associated with common cell configuration.
  void add_cell(const sched_cell_configuration_request_message& cell_cfg);

  /// Creates handles to the resources associated with a specific UE during its creation.
  ue_creation_params add_ue(const sched_ue_creation_request_message& creation_req);

  /// Creates handles to the resources associated with a specific UE during its reconfiguration.
  ue_reconfig_params reconf_ue(const sched_ue_reconfiguration_message& cfg_req);

private:
  logical_channel_config_pool lc_ch_pool;

  std::map<du_cell_index_t, du_cell_config_pool> cells;
};

} // namespace srsran
