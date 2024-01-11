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

#include "srsran/du_high/rnti_value_table.h"
#include "srsran/ran/du_types.h"
#include "srsran/ran/rnti.h"

namespace srsran {

using du_rnti_table = rnti_value_table<du_ue_index_t, du_ue_index_t::INVALID_DU_UE_INDEX>;

/// \brief Extends DU RNTI Table with the ability to allocate unique RNTIs for UEs.
class rnti_manager : public du_rnti_table
{
public:
  /// \brief Allocates new unique TC-RNTI.
  rnti_t allocate()
  {
    if (this->nof_ues() >= MAX_NOF_DU_UES) {
      // If the number of UEs is already maximum, ignore RACH.
      return rnti_t::INVALID_RNTI;
    }
    // Increments rnti counter until it finds an available temp C-RNTI.
    rnti_t temp_crnti;
    do {
      uint16_t prev_counter = rnti_counter.fetch_add(1, std::memory_order_relaxed) % CRNTI_RANGE;
      temp_crnti            = to_rnti(prev_counter + to_value(rnti_t::MIN_CRNTI));
    } while (this->has_rnti(temp_crnti));
    return temp_crnti;
  }

private:
  static constexpr int    CRNTI_RANGE  = to_value(rnti_t::MAX_CRNTI) + 1 - to_value(rnti_t::MIN_CRNTI);
  static constexpr rnti_t INITIAL_RNTI = to_rnti(0x4601);

  std::atomic<std::underlying_type_t<rnti_t>> rnti_counter{to_value(INITIAL_RNTI) - to_value(rnti_t::MIN_CRNTI)};
};

} // namespace srsran
