/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * This file is part of srsRAN.
 *
 * srsRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#include "../../../lib/phy/upper/upper_phy_rx_results_notifier_wrapper.h"
#include "../../../lib/phy/upper/upper_phy_rx_symbol_handler_impl.h"
#include "../support/prach_buffer_test_doubles.h"
#include "../support/resource_grid_test_doubles.h"
#include "uplink_processor_test_doubles.h"
#include "srsran/phy/support/prach_buffer_context.h"
#include "srsran/phy/upper/upper_phy_factories.h"
#include <gtest/gtest.h>

using namespace srsran;

namespace {

class UpperPhyRxSymbolHandlerFixture : public ::testing::Test
{
protected:
  static constexpr unsigned                  max_nof_prb    = 1;
  static constexpr unsigned                  max_nof_layers = 1;
  std::unique_ptr<rx_buffer_pool_controller> rm_buffer_pool;
  uplink_processor_spy*                      ul_proc_spy;
  std::unique_ptr<uplink_processor_pool>     ul_processor_pool;
  uplink_slot_pdu_repository                 pdu_repo;
  upper_phy_rx_results_notifier_wrapper      rx_results_wrapper;
  upper_phy_rx_symbol_handler_impl           rx_handler;
  prach_buffer_spy                           buffer_dummy;
  resource_grid_dummy                        rg;
  shared_resource_grid_spy                   shared_rg;

  void handle_prach_symbol()
  {
    prach_buffer_context prach_context;
    prach_context.sector = 0;
    prach_context.slot   = slot_point(0, 0, 0);

    rx_handler.handle_rx_prach_window(prach_context, buffer_dummy);
  }

  void handle_pusch_pdu()
  {
    const unsigned nof_symbols = 2;

    uplink_processor::pusch_pdu pdu = {};
    pdu.pdu.start_symbol_index      = 2;
    pdu.pdu.nof_symbols             = nof_symbols;
    pdu.pdu.cp                      = cyclic_prefix::NORMAL;
    pdu.pdu.codeword.emplace(pusch_processor::codeword_description{0, ldpc_base_graph_type::BG1, true});

    pdu_repo.add_pusch_pdu(slot_point(0, 0, 0), pdu);

    // Uplink processor gets called on the last symbol allocated in this PDU.
    for (unsigned i = 0, e = pdu.pdu.start_symbol_index + nof_symbols; i != e; ++i) {
      upper_phy_rx_symbol_context ctx = {};
      ctx.symbol                      = i;
      ctx.slot                        = slot_point(0, 0, 0);
      rx_handler.handle_rx_symbol(ctx, shared_rg.get_grid());
    }
  }

  void handle_late_pusch_pdu()
  {
    const unsigned nof_symbols = 2;

    uplink_processor::pusch_pdu pdu = {};
    pdu.pdu.start_symbol_index      = 2;
    pdu.pdu.nof_symbols             = nof_symbols;
    pdu.pdu.cp                      = cyclic_prefix::NORMAL;
    pdu.pdu.codeword.emplace(pusch_processor::codeword_description{0, ldpc_base_graph_type::BG1, true});

    pdu_repo.add_pusch_pdu(slot_point(0, 0, 0), pdu);

    // Uplink processor gets called for a slot that does not correspond with the PUSCH PDU.
    for (unsigned i = 0, e = pdu.pdu.start_symbol_index + nof_symbols; i != e; ++i) {
      upper_phy_rx_symbol_context ctx = {};
      ctx.symbol                      = i;
      ctx.slot                        = slot_point(0, 0, 1);
      rx_handler.handle_rx_symbol(ctx, shared_rg.get_grid());
    }
  }

  void handle_pucch_pdu()
  {
    static constexpr unsigned nof_symbols = 2;

    uplink_processor::pucch_pdu pdu     = {};
    auto&                       format0 = pdu.config.emplace<pucch_processor::format0_configuration>();
    format0.start_symbol_index          = 2;
    format0.nof_symbols                 = nof_symbols;
    format0.cp                          = cyclic_prefix::NORMAL;

    pdu_repo.add_pucch_pdu(slot_point(0, 0, 0), pdu);

    // Uplink processor gets called on the last symbol allocated in this PDU.
    for (unsigned i = 0, e = format0.start_symbol_index + nof_symbols; i != e; ++i) {
      upper_phy_rx_symbol_context ctx = {};
      ctx.symbol                      = i;
      ctx.slot                        = slot_point(0, 0, 0);
      rx_handler.handle_rx_symbol(ctx, shared_rg.get_grid());
    }
  }

  UpperPhyRxSymbolHandlerFixture() :
    rm_buffer_pool(create_rx_buffer_pool(rx_buffer_pool_config{16, 2, 2, 16})),
    ul_processor_pool(create_ul_processor_pool()),
    pdu_repo(2),
    rx_handler(*ul_processor_pool,
               pdu_repo,
               rm_buffer_pool->get_pool(),
               rx_results_wrapper,
               max_nof_prb,
               max_nof_layers),
    shared_rg(rg)
  {
    srslog::fetch_basic_logger("TEST").set_level(srslog::basic_levels::warning);
    srslog::init();
  }

  std::unique_ptr<uplink_processor_pool> create_ul_processor_pool()
  {
    uplink_processor_pool_config config_pool;
    config_pool.ul_processors.emplace_back();
    uplink_processor_pool_config::uplink_processor_set& info = config_pool.ul_processors.back();
    info.scs                                                 = subcarrier_spacing::kHz15;
    auto ul_proc                                             = std::make_unique<uplink_processor_spy>();
    ul_proc_spy                                              = &(*ul_proc);
    info.procs.push_back(std::move(ul_proc));

    return create_uplink_processor_pool(std::move(config_pool));
  }
};

} // namespace

TEST_F(UpperPhyRxSymbolHandlerFixture, handling_valid_prach_calls_uplink_processor)
{
  ASSERT_FALSE(ul_proc_spy->is_process_prach_method_called());

  handle_prach_symbol();

  ASSERT_TRUE(ul_proc_spy->is_process_prach_method_called());
}

TEST_F(UpperPhyRxSymbolHandlerFixture, handling_valid_pusch_pdu_calls_uplink_processor)
{
  ASSERT_FALSE(ul_proc_spy->is_process_pusch_method_called());

  handle_pusch_pdu();

  ASSERT_TRUE(ul_proc_spy->is_process_pusch_method_called());
}

TEST_F(UpperPhyRxSymbolHandlerFixture, handling_valid_pucch_pdu_calls_uplink_processor)
{
  ASSERT_FALSE(ul_proc_spy->is_process_pucch_method_called());

  handle_pucch_pdu();

  ASSERT_TRUE(ul_proc_spy->is_process_pucch_method_called());
}

TEST_F(UpperPhyRxSymbolHandlerFixture, handling_late_puxch_pdu_does_not_call_uplink_processor)
{
  ASSERT_FALSE(ul_proc_spy->is_process_pucch_method_called());

  handle_late_pusch_pdu();

  ASSERT_FALSE(ul_proc_spy->is_process_pucch_method_called());
}
