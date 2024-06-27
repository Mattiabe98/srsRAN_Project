/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "../../support/resource_grid_mapper_test_doubles.h"
#include "pdcch_modulator_test_data.h"
#include "srsran/phy/support/support_factories.h"
#include "srsran/phy/upper/channel_processors/channel_processor_factories.h"

using namespace srsran;

int main()
{
  std::shared_ptr<channel_modulation_factory> modulator_factory = create_channel_modulation_sw_factory();
  TESTASSERT(modulator_factory);

  std::shared_ptr<pseudo_random_generator_factory> prg_factory = create_pseudo_random_generator_sw_factory();
  TESTASSERT(prg_factory);

  std::shared_ptr<pdcch_modulator_factory> pdcch_factory =
      create_pdcch_modulator_factory_sw(modulator_factory, prg_factory);
  TESTASSERT(modulator_factory);

  std::unique_ptr<pdcch_modulator> pdcch = pdcch_factory->create();
  TESTASSERT(pdcch);

  for (const test_case_t& test_case : pdcch_modulator_test_data) {
    TESTASSERT(test_case.config.rb_mask.count() > 1);
    int      prb_idx_high = test_case.config.rb_mask.find_highest();
    unsigned max_prb      = static_cast<unsigned>(prb_idx_high + 1);
    unsigned max_symb     = test_case.config.start_symbol_index + test_case.config.duration;
    unsigned max_ports    = test_case.config.precoding.get_nof_ports();

    // Prepare resource grid and resource grid mapper spies.
    resource_grid_writer_spy              grid(max_ports, max_symb, max_prb);
    std::unique_ptr<resource_grid_mapper> mapper = create_resource_grid_mapper(max_ports, NRE * max_prb, grid);

    // Load input codeword from a testvector
    const std::vector<uint8_t> test_codeword = test_case.data.read();

    // Modulate.
    pdcch->modulate(*mapper, test_codeword, test_case.config);

    // Load output golden data
    const std::vector<resource_grid_writer_spy::expected_entry_t> testvector_symbols = test_case.symbols.read();

    // Tolerance: max BF16 error times sqrt(2), since we are taking the modulus.
    constexpr float tolerance = M_SQRT2f32 / 256.0;
    // Assert resource grid entries.
    grid.assert_entries(testvector_symbols, tolerance);
  }

  return 0;
}
