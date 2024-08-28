/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "e2sm_rc_control_action_cu_executor.h"
#include <future>
using namespace asn1::e2ap;
using namespace asn1::e2sm;
using namespace srsran;

e2sm_rc_control_action_cu_executor_base::e2sm_rc_control_action_cu_executor_base(cu_configurator& cu_configurator_,
                                                                                 uint32_t         action_id_) :
  logger(srslog::fetch_basic_logger("E2SM-RC")), action_id(action_id_), cu_param_configurator(cu_configurator_)
{
}

uint32_t e2sm_rc_control_action_cu_executor_base::get_action_id()
{
  return action_id;
}

bool e2sm_rc_control_action_cu_executor_base::fill_ran_function_description(
    asn1::e2sm::ran_function_definition_ctrl_action_item_s& action_item)
{
  action_item.ric_ctrl_action_id = action_id;
  action_item.ric_ctrl_action_name.from_string(action_name);

  for (auto& ran_p : action_params) {
    ctrl_action_ran_param_item_s ctrl_action_ran_param_item;
    ctrl_action_ran_param_item.ran_param_id = ran_p.first;
    ctrl_action_ran_param_item.ran_param_name.from_string(ran_p.second);
    action_item.ran_ctrl_action_params_list.push_back(ctrl_action_ran_param_item);
  }

  return true;
}
async_task<e2sm_ric_control_response>
e2sm_rc_control_action_cu_executor_base::return_ctrl_failure(const e2sm_ric_control_request& req)
{
  return launch_async([req](coro_context<async_task<e2sm_ric_control_response>>& ctx) {
    CORO_BEGIN(ctx);
    e2sm_ric_control_response e2sm_response;
    e2sm_response.success                = false;
    e2sm_response.cause.set_misc().value = cause_misc_e::options::unspecified;
    CORO_RETURN(e2sm_response);
  });
}

e2sm_rc_control_action_3_1_cu_executor::e2sm_rc_control_action_3_1_cu_executor(cu_configurator& cu_configurator_) :
  e2sm_rc_control_action_cu_executor_base(cu_configurator_, 1)
{
}

bool e2sm_rc_control_action_3_1_cu_executor::ric_control_action_supported(const e2sm_ric_control_request& req)
{
  return true;
}

async_task<e2sm_ric_control_response>
e2sm_rc_control_action_3_1_cu_executor::execute_ric_control_action(const e2sm_ric_control_request& req)
{
  return launch_async([this, req](coro_context<async_task<e2sm_ric_control_response>>& ctx) {
    CORO_BEGIN(ctx);
    e2sm_ric_control_response e2sm_response;
    // cu_param_configurator.get_e2_mobility_notifier().on_inter_du_handover_required();
    e2sm_response.success = true;
    CORO_RETURN(e2sm_response);
  });
}

void e2sm_rc_control_action_3_1_cu_executor::parse_action_ran_parameter_value(
    const asn1::e2sm::ran_param_value_type_c& ran_p,
    uint64_t                                  ran_param_id,
    uint64_t                                  ue_id,
    cu_handover_control_config&               ctrl_cfg)
{
}
