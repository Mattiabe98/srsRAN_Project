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

#include "srsran/adt/byte_buffer.h"
#include "srsran/cu_cp/cu_cp_types.h"
#include "srsran/ran/positioning/measurement_information.h"
#include "srsran/ran/positioning/trp_information_exchange.h"
#include "srsran/ran/rb_id.h"
#include "srsran/support/async/async_task.h"

namespace srsran {
namespace srs_cu_cp {

/// Handles incoming NRPPa messages to be transmitted over the F1 interface.
class f1ap_nrppa_message_handler
{
public:
  virtual ~f1ap_nrppa_message_handler() = default;

  /// Handles the given TRP information request message.
  virtual async_task<expected<trp_information_response_t, trp_information_failure_t>>
  handle_trp_information_request(const trp_information_request_t& request) = 0;

  /// Handles the given measurement information request message.
  virtual async_task<expected<measurement_response_t, measurement_failure_t>>
  handle_measurement_information_request(const measurement_request_t& request) = 0;
};

} // namespace srs_cu_cp
} // namespace srsran
