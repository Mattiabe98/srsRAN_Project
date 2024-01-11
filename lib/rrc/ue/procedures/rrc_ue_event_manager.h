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

#include "srsran/adt/expected.h"
#include "srsran/asn1/rrc_nr/rrc_nr.h"
#include "srsran/support/async/event_signal.h"
#include "srsran/support/async/protocol_transaction_manager.h"

namespace srsran {
namespace srs_cu_cp {

using rrc_outcome     = asn1::rrc_nr::ul_dcch_msg_s;
using rrc_transaction = protocol_transaction<rrc_outcome>;

/// TODO: Do we need a separate class for each procedure? Note that the outcome's expected only has two choices.
class rrc_ue_event_manager
{
public:
  /// Transaction Response Container, which gets indexed by transaction_id.
  constexpr static size_t                   MAX_NOF_TRANSACTIONS = 4; // Two bit RRC transaction id
  protocol_transaction_manager<rrc_outcome> transactions;

  explicit rrc_ue_event_manager(timer_factory timers) : transactions(MAX_NOF_TRANSACTIONS, timers) {}
};

} // namespace srs_cu_cp
} // namespace srsran
