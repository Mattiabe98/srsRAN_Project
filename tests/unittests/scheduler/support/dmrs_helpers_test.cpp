/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "lib/scheduler/support/dmrs_helpers.h"
#include "srsran/ran/pci.h"
#include "srsran/scheduler/config/bwp_configuration.h"
#include "srsran/support/srsran_test.h"

using namespace srsran;

// Test bench to hold DMRS symbol masks, based on Table 7.4.1.1.2-3, TS 38.211.
struct test_bench {
  void generate_dmrs_symb_masks(dmrs_typeA_position typeA_pos)
  {
    size_t l0 = static_cast<unsigned>(typeA_pos);
    // Mask with bit at position set.
    mask_l0.resize(14);
    mask_l0.reset();
    mask_l0.set(l0);

    // Mask with bits at position l0 and 7 set.
    mask_l0_7.resize(14);
    mask_l0_7.reset();
    mask_l0_7.set(l0);
    mask_l0_7.set(7);

    // Mask with bits at position l0, 6, 9 set.
    mask_l0_6_9.resize(14);
    mask_l0_6_9.reset();
    mask_l0_6_9.set(l0);
    mask_l0_6_9.set(6);
    mask_l0_6_9.set(9);

    // Mask with bits at position l0, 7, 11 set.
    mask_l0_7_11.resize(14);
    mask_l0_7_11.reset();
    mask_l0_7_11.set(l0);
    mask_l0_7_11.set(7);
    mask_l0_7_11.set(11);
  }

  dmrs_symbol_mask mask_l0, mask_l0_7, mask_l0_6_9, mask_l0_7_11;
};

// Helper that generates the expeceted dmrs_information.
dmrs_information generate_expected_dmrs_info(const dmrs_symbol_mask& mask, unsigned symb_length, pci_t pci)
{
  uint8_t          num_dmrs_cdm_grps = symb_length == 2 ? 1 : 2;
  dmrs_information dmrs{.config_type               = dmrs_config_type::type1,
                        .dmrs_scrambling_id        = pci,
                        .low_papr_dmrs             = false,
                        .n_scid                    = false,
                        .num_dmrs_cdm_grps_no_data = num_dmrs_cdm_grps};
  dmrs.dmrs_symb_pos = mask;
  dmrs.dmrs_ports.resize(12);
  dmrs.dmrs_ports.set(0);

  return dmrs;
}

// Helper to compare the test dmrs_information with the expected one.
void verify_dmrs_info(const dmrs_information& expected_dmrs, const dmrs_information& dmrs_under_test)
{
  TESTASSERT(expected_dmrs.dmrs_symb_pos == dmrs_under_test.dmrs_symb_pos);
  TESTASSERT_EQ(expected_dmrs.config_type, dmrs_under_test.config_type);
  TESTASSERT_EQ(expected_dmrs.dmrs_scrambling_id, dmrs_under_test.dmrs_scrambling_id);
  TESTASSERT_EQ(expected_dmrs.low_papr_dmrs, dmrs_under_test.low_papr_dmrs);
  TESTASSERT_EQ(expected_dmrs.n_scid, dmrs_under_test.n_scid);
  TESTASSERT_EQ(expected_dmrs.num_dmrs_cdm_grps_no_data, dmrs_under_test.num_dmrs_cdm_grps_no_data);
  TESTASSERT(expected_dmrs.dmrs_ports == dmrs_under_test.dmrs_ports);
}

/// This test checks if the dmrs_information generated by the function make_dmrs_info_common() is correct.
/// The dmrs_information depends on: (i) dmrs_typeA_position; (ii) PDSCH time_domain allocation symbols.
void test_make_dmrs_info_common()
{
  test_bench t_bench;
  pci_t      pci = 1;

  // Iterate this over dmrs_typeA_position, i.e., pos2 and pos3.
  std::array<dmrs_typeA_position, 2> dmrs_typeA_position_choices{dmrs_typeA_position::pos2, dmrs_typeA_position::pos3};
  for (auto dmrs_typeA_pos : dmrs_typeA_position_choices) {
    // Generate DMRS symbol mask mapping.
    t_bench.generate_dmrs_symb_masks(dmrs_typeA_pos);

    // Iterate this over the start Symbol S from 0 until 3.
    for (uint8_t S = 0; S <= 3; ++S) {
      // Generate symbols list for with all possible combinations [S, S+L) of L for a given S. These all these
      // combinations can fit in the pdsch_cfg.pdsch_td_alloc_list.
      pdsch_config_common pdsch_cfg{};
      for (uint8_t L = 3; L <= 14; ++L) {
        if (S + L > 14) {
          continue;
        }

        pdsch_cfg.pdsch_td_alloc_list.push_back(pdsch_time_domain_resource_allocation{
            .k0 = 0, .map_type = sch_mapping_type::typeA, .symbols = ofdm_symbol_range{S, S + L}});
      }

      // Verify that for each symbol set [S, S+L) in pdsch_cfg.pdsch_td_alloc_list the function generates the correct
      // struct.
      for (uint8_t idx = 0; idx < pdsch_cfg.pdsch_td_alloc_list.size(); ++idx) {
        // Generate dmrs_information to be tested.
        const dmrs_information test_dmrs =
            make_dmrs_info_common(pdsch_cfg.pdsch_td_alloc_list, idx, pci, dmrs_typeA_pos);

        // As per TS 38.211, Section 7.4.1.1.2, "for PDSCH mapping type A, l_d is the duration between the first OFDM
        // symbol of the slot and the last OFDM symbol of the scheduled PDSCH resources in the slot".
        unsigned l_d            = pdsch_cfg.pdsch_td_alloc_list[idx].symbols.stop();
        unsigned pdsch_duration = pdsch_cfg.pdsch_td_alloc_list[idx].symbols.length();

        // According to Table 7.4.1.1.2-3, TS 38.211 (PDSCH mapping type A and dmrs-AdditionalPosition == pos2, which is
        // what the function under test make_dmrs_info_common() assumes), dmrs_symbols are different depending on l_d.
        // Generate the expected dmrs_information and compare it with the resul of the tested function.
        if (l_d > 2 and l_d <= 7) {
          dmrs_information expected_dmrs = generate_expected_dmrs_info(t_bench.mask_l0, pdsch_duration, pci);
          verify_dmrs_info(expected_dmrs, test_dmrs);
        } else if (l_d > 7 and l_d <= 9) {
          dmrs_information expected_dmrs = generate_expected_dmrs_info(t_bench.mask_l0_7, pdsch_duration, pci);
          verify_dmrs_info(expected_dmrs, test_dmrs);
        } else if (l_d > 9 and l_d <= 12) {
          dmrs_information expected_dmrs = generate_expected_dmrs_info(t_bench.mask_l0_6_9, pdsch_duration, pci);
          verify_dmrs_info(expected_dmrs, test_dmrs);
        } else if (l_d > 12 and l_d <= 14) {
          dmrs_information expected_dmrs = generate_expected_dmrs_info(t_bench.mask_l0_7_11, pdsch_duration, pci);
          verify_dmrs_info(expected_dmrs, test_dmrs);
        }
      }
    }
  }
}

int main()
{
  test_make_dmrs_info_common();
  return 0;
}
