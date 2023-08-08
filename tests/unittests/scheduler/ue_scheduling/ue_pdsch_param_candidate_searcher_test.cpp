/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "../test_utils/config_generators.h"
#include "../test_utils/dummy_test_components.h"
#include "lib/scheduler/ue_scheduling/ue_pdsch_param_candidate_searcher.h"
#include <gtest/gtest.h>

using namespace srsran;

class ue_pdsch_param_candidate_searcher_test : public ::testing::Test
{
protected:
  ue_pdsch_param_candidate_searcher_test()
  {
    // Create UE.
    sched_ue_creation_request_message ue_creation_req = test_helpers::create_default_sched_ue_creation_request();
    ue_creation_req.ue_index                          = to_du_ue_index(0);
    ue_creation_req.crnti                             = to_rnti(0x4601 + (unsigned)ue_creation_req.ue_index);
    for (lcid_t lcid : std::array<lcid_t, 3>{uint_to_lcid(1), uint_to_lcid(2), uint_to_lcid(4)}) {
      ue_creation_req.cfg.lc_config_list.push_back(config_helpers::create_default_logical_channel_config(lcid));
    }
    ue_ptr = std::make_unique<ue>(expert_cfg, cell_cfg, ue_creation_req, harq_timeout_handler);
    ue_cc  = &ue_ptr->get_cell(to_ue_cell_index(0));
  }

  scheduler_ue_expert_config           expert_cfg = config_helpers::make_default_scheduler_expert_config().ue;
  cell_configuration                   cell_cfg{test_helpers::make_default_sched_cell_configuration_request()};
  scheduler_harq_timeout_dummy_handler harq_timeout_handler;

  std::unique_ptr<ue> ue_ptr;
  ue_cell*            ue_cc = nullptr;
};

TEST_F(ue_pdsch_param_candidate_searcher_test, when_no_pending_newtx_bytes_then_zero_candidates)
{
  ue_pdsch_param_candidate_searcher searcher(*ue_ptr, to_ue_cell_index(0), false, slot_point{0, 0});
  EXPECT_EQ(searcher.begin(), searcher.end());
}

TEST_F(ue_pdsch_param_candidate_searcher_test, when_no_pending_retxs_then_zero_candidates)
{
  ue_pdsch_param_candidate_searcher searcher2(*ue_ptr, to_ue_cell_index(0), true, slot_point{0, 0});
  EXPECT_EQ(searcher2.begin(), searcher2.end());
}

TEST_F(ue_pdsch_param_candidate_searcher_test, when_pending_newtx_bytes_then_there_are_newtx_candidates)
{
  ue_ptr->handle_dl_buffer_state_indication(dl_buffer_state_indication_message{ue_ptr->ue_index, uint_to_lcid(4), 100});

  ue_pdsch_param_candidate_searcher searcher(*ue_ptr, to_ue_cell_index(0), false, slot_point{0, 0});
  EXPECT_NE(searcher.begin(), searcher.end());

  std::set<harq_id_t> harqs_visited;
  for (const ue_pdsch_param_candidate_searcher::candidate& candidate : searcher) {
    harqs_visited.insert(candidate.harq().id);
    EXPECT_TRUE(candidate.harq().empty());
    ASSERT_EQ(&ue_cc->harqs.dl_harq(candidate.harq().id), &candidate.harq());
    EXPECT_EQ(&ue_cc->cfg().search_space(candidate.ss().cfg->get_id()), &candidate.ss());
    EXPECT_LT(candidate.pdsch_td_res_index(), candidate.ss().pdsch_time_domain_list.size());
  }
  ASSERT_EQ(harqs_visited.size(), 1) << "In case of newTx, we should not iterate through multiple HARQs";
}
