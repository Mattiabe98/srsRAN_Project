/*
 *
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "srsran/f1u/cu_up/split_connector/f1u_split_connector_factory.h"
#include "f1u_split_connector.h"

using namespace srsran;
using namespace srs_cu_up;

std::unique_ptr<f1u_cu_up_udp_gateway> srsran::srs_cu_up::create_split_f1u_gw(f1u_cu_up_split_gateway_creation_msg msg)
{
  f1u_session_manager* f1u_session_mngr = nullptr; // TODO fix this and pass in the f1u_cu_up_split_gateway_creation_msg
  return std::make_unique<f1u_split_connector>(
      *f1u_session_mngr, msg.udp_gw, msg.demux, msg.gtpu_pcap, msg.peer_port, msg.f1u_ext_addr);
}
