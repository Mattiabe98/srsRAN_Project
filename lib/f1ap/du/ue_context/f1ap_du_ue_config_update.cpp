/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "f1ap_du_ue_config_update.h"

using namespace srsran;
using namespace srs_du;

f1ap_ue_creation_response srsran::srs_du::create_f1ap_ue(const f1ap_ue_creation_request& req,
                                                         f1ap_du_ue_manager&             ues,
                                                         const f1ap_du_context&          du_ctx,
                                                         f1ap_event_manager&             ev_mng)
{
  f1ap_du_ue& u                        = ues.add_ue(req.ue_index);
  u.context.rnti                       = req.c_rnti;
  const f1ap_du_cell_context& ue_pcell = du_ctx.served_cells[req.pcell_index];

  // Create an F1c bearer for each requested SRB.
  for (const f1c_bearer_to_addmod& srb : req.f1c_bearers_to_add) {
    if (srb.srb_id == srb_id_t::srb0) {
      u.bearers.add_srb0_f1c_bearer(*srb.rx_sdu_notifier, ue_pcell.nr_cgi, req.du_cu_rrc_container, ev_mng);
    } else {
      u.bearers.add_f1c_bearer(srb.srb_id, *srb.rx_sdu_notifier);
    }
  }

  // Prepare response.
  f1ap_ue_creation_response resp{};
  resp.result = true;
  for (const f1c_bearer_to_addmod& srb : req.f1c_bearers_to_add) {
    resp.f1c_bearers_added.push_back(u.bearers.find_srb(srb.srb_id));
  }
  return resp;
}

f1ap_ue_configuration_response srsran::srs_du::update_f1ap_ue_config(const f1ap_ue_configuration_request& req,
                                                                     f1ap_du_ue_manager&                  ues)
{
  f1ap_du_ue& u = ues[req.ue_index];

  // Add F1-C bearers.
  for (const f1c_bearer_to_addmod& srb : req.f1c_bearers_to_add) {
    u.bearers.add_f1c_bearer(srb.srb_id, *srb.rx_sdu_notifier);
  }

  // Prepare response.
  f1ap_ue_configuration_response resp;
  for (const f1c_bearer_to_addmod& srb : req.f1c_bearers_to_add) {
    resp.f1c_bearers_added.emplace_back();
    resp.f1c_bearers_added.back().srb_id = srb.srb_id;
    resp.f1c_bearers_added.back().bearer = u.bearers.find_srb(srb.srb_id);
  }

  return resp;
}
