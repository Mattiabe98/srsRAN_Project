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

#include "ngu_session_manager.h"
#include "ue_manager_interfaces.h"
#include "srsran/f1u/cu_up/f1u_gateway.h"
#include "srsran/gtpu/gtpu_teid_pool.h"
#include "srsran/support/async/fifo_async_task_scheduler.h"
#include "srsran/support/timers.h"
#include <unordered_map>

namespace srsran {

namespace srs_cu_up {

/// UE manager configuration.
struct ue_manager_config {
  const n3_interface_config&    n3_config;
  const cu_up_test_mode_config& test_mode_config;
};

/// UE manager dependencies.
struct ue_manager_dependencies {
  e1ap_control_message_handler& e1ap;
  timer_manager&                timers;
  f1u_cu_up_gateway&            f1u_gw;
  ngu_session_manager&          ngu_session_mngr;
  gtpu_demux_ctrl&              gtpu_rx_demux;
  gtpu_teid_pool&               n3_teid_allocator;
  gtpu_teid_pool&               f1u_teid_allocator;
  cu_up_executor_mapper&        exec_pool;
  dlt_pcap&                     gtpu_pcap;
  srslog::basic_logger&         logger;
};

class ue_manager : public ue_manager_ctrl
{
public:
  explicit ue_manager(const ue_manager_config& config, const ue_manager_dependencies& dependencies);

  using ue_db_t              = std::unordered_map<ue_index_t, std::unique_ptr<ue_context>>;
  using ue_task_schedulers_t = slotted_array<fifo_async_task_scheduler, MAX_NOF_UES>;
  const ue_db_t& get_ues() const { return ue_db; }

  async_task<void> stop() override;
  ue_context*      add_ue(const ue_context_cfg& cfg) override;
  async_task<void> remove_ue(ue_index_t ue_index) override;
  ue_context*      find_ue(ue_index_t ue_index) override;
  size_t           get_nof_ues() const override { return ue_db.size(); };

  void schedule_ue_async_task(ue_index_t ue_index, async_task<void> task);

private:
  /// \brief Get the next available UE index.
  /// \return The UE index.
  ue_index_t get_next_ue_index();

  const n3_interface_config&    n3_config;
  const cu_up_test_mode_config& test_mode_config;
  e1ap_control_message_handler& e1ap;
  f1u_cu_up_gateway&            f1u_gw;
  ngu_session_manager&          ngu_session_mngr;
  gtpu_demux_ctrl&              gtpu_rx_demux;
  gtpu_teid_pool&               n3_teid_allocator;
  gtpu_teid_pool&               f1u_teid_allocator;
  cu_up_executor_mapper&        exec_pool;
  task_executor&                ctrl_executor;
  dlt_pcap&                     gtpu_pcap;
  timer_manager&                timers;
  ue_db_t                       ue_db;
  ue_task_schedulers_t          ue_task_schedulers;
  srslog::basic_logger&         logger;
};

} // namespace srs_cu_up

} // namespace srsran
