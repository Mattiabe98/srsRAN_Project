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

#include "srsran/f1u/du/f1u_gateway.h"

namespace srsran {
namespace srs_du {

/// \brief Dummy F1-U bearer for the purpose of benchmark.
class f1u_dummy_bearer : public f1u_bearer,
                         public f1u_rx_pdu_handler,
                         public f1u_tx_delivery_handler,
                         public f1u_tx_sdu_handler
{
public:
  f1u_rx_pdu_handler&      get_rx_pdu_handler() override { return *this; }
  f1u_tx_delivery_handler& get_tx_delivery_handler() override { return *this; }
  f1u_tx_sdu_handler&      get_tx_sdu_handler() override { return *this; }

  void handle_pdu(nru_dl_message msg) override {}
  void handle_transmit_notification(uint32_t highest_pdcp_sn) override {}
  void handle_delivery_notification(uint32_t highest_pdcp_sn) override {}
  void handle_sdu(byte_buffer_chain sdu) override {}
};

/// \brief Simulator of the CU-UP from the perspective of the DU.
class cu_up_simulator : public f1u_du_gateway
{
public:
  f1u_dummy_bearer             bearer;
  srs_du::f1u_rx_sdu_notifier* du_notif = nullptr;

  f1u_bearer* create_du_bearer(uint32_t                       ue_index,
                               drb_id_t                       drb_id,
                               srs_du::f1u_config             config,
                               const up_transport_layer_info& dl_tnl,
                               const up_transport_layer_info& ul_tnl,
                               srs_du::f1u_rx_sdu_notifier&   du_rx,
                               timer_factory                  timers) override
  {
    du_notif = &du_rx;
    return &bearer;
  }

  void remove_du_bearer(const up_transport_layer_info& dl_tnl) override { du_notif = nullptr; }
};

} // namespace srs_du
} // namespace srsran