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

#include "srsran/adt/byte_buffer.h"
#include "srsran/asn1/asn1_utils.h"
#include "srsran/asn1/e2ap/e2ap.h"
#include "srsran/asn1/e2ap/e2sm_kpm.h"
#include "srsran/e2/e2sm/e2sm.h"
#include "srsran/e2/e2sm/e2sm_kpm.h"

namespace srsran {

class e2sm_kpm_asn1_packer : public e2sm_handler
{
public:
  static const std::string short_name;
  static const std::string oid;
  static const std::string func_description;
  static const uint32_t    ran_func_id;
  static const uint32_t    revision;

  e2sm_kpm_asn1_packer(e2sm_kpm_meas_provider& meas_provider_);

  /// Receive populated ASN1 struct that needs to be unpacked and forwarded.
  e2sm_action_definition   handle_packed_e2sm_action_definition(const srsran::byte_buffer& action_definition) override;
  e2sm_ric_control_request handle_packed_ric_control_request(const asn1::e2ap::ri_cctrl_request_s& req) override;
  e2_ric_control_response  pack_ric_control_response(const e2sm_ric_control_response& e2sm_response) override;

  e2sm_event_trigger_definition
  handle_packed_event_trigger_definition(const srsran::byte_buffer& event_trigger_definition) override;

  asn1::unbounded_octstring<true> pack_ran_function_description() override;

private:
  e2sm_kpm_meas_provider& meas_provider;
};
} // namespace srsran
