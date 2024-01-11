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

#include "srsran/du_manager/du_manager.h"
#include "srsran/f1ap/du/f1ap_du.h"
#include "srsran/support/async/execute_on.h"
#include "srsran/support/timers.h"

namespace srsran {

class timer_manager;

namespace srs_du {

class f1ap_ue_task_scheduler_adapter final : public f1ap_ue_task_scheduler
{
public:
  explicit f1ap_ue_task_scheduler_adapter(du_ue_index_t ue_index_, timer_factory timers_) :
    ue_index(ue_index_), timers(timers_)
  {
  }

  unique_timer create_timer() override { return timers.create_timer(); }

  void schedule_async_task(async_task<void>&& task) override { du_mng->schedule_async_task(ue_index, std::move(task)); }

  void connect(du_manager_configurator& du_mng_) { du_mng = &du_mng_; }

private:
  du_ue_index_t            ue_index;
  timer_factory            timers;
  du_manager_configurator* du_mng = nullptr;
};

class f1ap_du_configurator_adapter : public f1ap_du_configurator
{
public:
  explicit f1ap_du_configurator_adapter(timer_factory timers_) : timers(timers_)
  {
    for (unsigned i = 0; i != MAX_NOF_DU_UES; ++i) {
      ues.emplace(i, to_du_ue_index(i), timers);
    }
  }

  void connect(du_manager_configurator& du_mng_)
  {
    du_mng = &du_mng_;
    for (unsigned i = 0; i != MAX_NOF_DU_UES; ++i) {
      ues[i].connect(*du_mng);
    }
  }

  timer_factory& get_timer_factory() override { return timers; }

  void schedule_async_task(async_task<void>&& task) override { du_mng->schedule_async_task(std::move(task)); }

  f1ap_ue_task_scheduler& get_ue_handler(du_ue_index_t ue_index) override { return ues[ue_index]; }

  du_ue_index_t find_free_ue_index() override { return du_mng->find_unused_du_ue_index(); }

  async_task<f1ap_ue_context_creation_response>
  request_ue_creation(const f1ap_ue_context_creation_request& request) override
  {
    return du_mng->handle_ue_context_creation(request);
  }

  async_task<f1ap_ue_context_update_response>
  request_ue_context_update(const f1ap_ue_context_update_request& request) override
  {
    return du_mng->handle_ue_context_update(request);
  }

  async_task<void> request_ue_removal(const f1ap_ue_delete_request& request) override
  {
    return du_mng->handle_ue_delete_request(request);
  }

  void notify_reestablishment_of_old_ue(du_ue_index_t new_ue_index, du_ue_index_t old_ue_index) override
  {
    du_mng->handle_ue_reestablishment(new_ue_index, old_ue_index);
  }

private:
  timer_factory                                                 timers;
  du_manager_configurator*                                      du_mng = nullptr;
  slotted_array<f1ap_ue_task_scheduler_adapter, MAX_NOF_DU_UES> ues;
};

} // namespace srs_du
} // namespace srsran
