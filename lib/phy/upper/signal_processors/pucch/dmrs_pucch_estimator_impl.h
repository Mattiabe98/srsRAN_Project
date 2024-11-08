/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

/// \file
/// \brief DM-RS PUCCH estimator implementation declaration.

#pragma once

#include "dmrs_pucch_estimator_format1.h"
#include "dmrs_pucch_estimator_format2.h"
#include "dmrs_pucch_estimator_formats_3_4.h"
#include "srsran/phy/upper/signal_processors/dmrs_pucch_estimator.h"

namespace srsran {

/// DM-RS PUCCH estimator implementation.
class dmrs_pucch_estimator_impl : public dmrs_pucch_estimator
{
public:
  /// Constructor: sets up internal components and acquires their ownership.
  /// \param[in] estimator_format1_ PUCCH Format 1 DM-RS estimator.
  /// \param[in] estimator_format2_ PUCCH Format 1 DM-RS estimator.
  /// \param[in] estimator_formats_3_4_ PUCCH Format 3 and Format 4 DM-RS estimator.
  dmrs_pucch_estimator_impl(std::unique_ptr<dmrs_pucch_estimator_format1>     estimator_format1_,
                            std::unique_ptr<dmrs_pucch_estimator_format2>     estimator_format2_,
                            std::unique_ptr<dmrs_pucch_estimator_formats_3_4> estimator_formats_3_4_) :
    estimator_format1(std::move(estimator_format1_)),
    estimator_format2(std::move(estimator_format2_)),
    estimator_formats_3_4(std::move(estimator_formats_3_4_))
  {
    srsran_assert(estimator_format1, "Invalid pointer to dmrs_pucch_estimator_format1 object.");
    srsran_assert(estimator_format2, "Invalid pointer to dmrs_pucch_estimator_format2 object.");
    srsran_assert(estimator_formats_3_4, "Invalid pointer to dmrs_pucch_estimator_formats_3_4 object.");
  }

  // See interface for the documentation.
  void
  estimate(channel_estimate& estimate, const resource_grid_reader& grid, const format1_configuration& config) override
  {
    estimator_format1->estimate(estimate, grid, config);
  }

  // See interface for the documentation.
  void
  estimate(channel_estimate& estimate, const resource_grid_reader& grid, const format2_configuration& config) override
  {
    estimator_format2->estimate(estimate, grid, config);
  }

  // See interface for the documentation.
  void
  estimate(channel_estimate& estimate, const resource_grid_reader& grid, const format3_configuration& config) override
  {
    estimator_formats_3_4->estimate(estimate, grid, config);
  }

  // See interface for the documentation.
  void
  estimate(channel_estimate& estimate, const resource_grid_reader& grid, const format4_configuration& config) override
  {
    estimator_formats_3_4->estimate(estimate, grid, config);
  }

private:
  /// PUCCH DM-RS estimator Format 1 component.
  std::unique_ptr<dmrs_pucch_estimator_format1> estimator_format1;
  /// PUCCH DM-RS estimator Format 1 component.
  std::unique_ptr<dmrs_pucch_estimator_format2> estimator_format2;
  /// PUCCH DM-RS estimator Format 3 and Format 4 component.
  std::unique_ptr<dmrs_pucch_estimator_formats_3_4> estimator_formats_3_4;
};

} // namespace srsran
