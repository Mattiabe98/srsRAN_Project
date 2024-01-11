/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "pdcp_tx_status_report_test.h"
#include "pdcp_test_vectors.h"
#include "srsran/pdcp/pdcp_config.h"
#include "srsran/support/bit_encoding.h"
#include "srsran/support/test_utils.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <list>
#include <queue>

using namespace srsran;

/// Test correct handling of PDCP status report
TEST_P(pdcp_tx_status_report_test, handle_status_report)
{
  init(GetParam());
  auto test_with_count = [this](uint32_t tx_next) {
    // clear queue from previous runs
    test_frame.sdu_discard_queue   = {};
    unsigned                n_sdus = 5;
    std::queue<pdcp_tx_pdu> exp_pdu_list;
    pdcp_tx_state           st = {tx_next, tx_next};
    pdcp_tx->set_state(st);
    pdcp_tx->configure_security(sec_cfg);
    srsran::test_delimit_logger delimiter("Testing data recovery. SN_SIZE={} COUNT={}", sn_size, tx_next);
    for (uint32_t count = tx_next; count < tx_next + n_sdus; ++count) {
      // Write SDU
      byte_buffer sdu = {sdu1};
      pdcp_tx->handle_sdu(std::move(sdu));
      pdcp_tx->handle_transmit_notification(pdcp_compute_sn(count, sn_size));

      // Get generated PDU
      ASSERT_EQ(test_frame.pdu_queue.size(), 1);
      exp_pdu_list.push(std::move(test_frame.pdu_queue.front()));
      test_frame.pdu_queue.pop();
      ASSERT_EQ(test_frame.pdu_queue.size(), 0);
    }

    ASSERT_TRUE(test_frame.sdu_discard_queue.empty());

    // Build status report that confirms RX of SDUs:
    // Confirm: [tx_next,            tx_next+2,            tx_next+4]
    // Missing: [         tx_next+1,            tx_next+3,          ]
    //
    // Report:            [  FMC  ][    1          0          1     ][  0   0   0   0   0  ]
    //                    |-first-||--------- bitmap ---------------;;-- bitmap padding ---|
    byte_buffer buf = {};
    bit_encoder enc(buf);

    // Pack PDU header
    enc.pack(to_number(pdcp_dc_field::control), 1);
    enc.pack(to_number(pdcp_control_pdu_type::status_report), 3);
    enc.pack(0b0000, 4);

    // Pack tx_next+1 into FMC field
    enc.pack(tx_next + 1, 32);

    // Pack bitmap
    enc.pack(0b10100000, 8);

    // Handle this status report
    pdcp_tx->on_status_report(byte_buffer_chain{std::move(buf)});

    // Verify discard
    {
      std::array<uint32_t, 3> exp_pdcp_sns = {pdcp_compute_sn(tx_next, sn_size),
                                              pdcp_compute_sn(tx_next + 2, sn_size),
                                              pdcp_compute_sn(tx_next + 4, sn_size)};
      ASSERT_FALSE(test_frame.sdu_discard_queue.empty());
      while (not test_frame.sdu_discard_queue.empty()) {
        ASSERT_NE(std::find(exp_pdcp_sns.begin(), exp_pdcp_sns.end(), test_frame.sdu_discard_queue.front()),
                  exp_pdcp_sns.end());
        test_frame.sdu_discard_queue.pop();
      }
    }

    // Let timers expire
    for (int i = 0; i < 10; i++) {
      timers.tick();
      worker.run_pending_tasks();
    }

    // Verify discard timers for already discarded SDU were disarmed (no duplicated discard)
    {
      std::array<uint32_t, 2> exp_pdcp_sns = {pdcp_compute_sn(tx_next + 1, sn_size),
                                              pdcp_compute_sn(tx_next + 3, sn_size)};
      ASSERT_FALSE(test_frame.sdu_discard_queue.empty());
      while (not test_frame.sdu_discard_queue.empty()) {
        ASSERT_NE(std::find(exp_pdcp_sns.begin(), exp_pdcp_sns.end(), test_frame.sdu_discard_queue.front()),
                  exp_pdcp_sns.end());
        test_frame.sdu_discard_queue.pop();
      }
    }
  };

  if (sn_size == pdcp_sn_size::size12bits) {
    test_with_count(0);
    test_with_count(2048);
    test_with_count(4096);
  } else if (sn_size == pdcp_sn_size::size18bits) {
    test_with_count(0);
    test_with_count(131072);
    test_with_count(262144);
  } else {
    FAIL();
  }
}

/// \brief Test basic data recovery functionality
TEST_P(pdcp_tx_status_report_test, data_recovery)
{
  init(GetParam());
  auto test_with_count = [this](uint32_t tx_next) {
    unsigned                n_sdus = 5;
    std::queue<pdcp_tx_pdu> exp_pdu_list;
    pdcp_tx_state           st = {tx_next, tx_next};
    pdcp_tx->set_state(st);
    pdcp_tx->configure_security(sec_cfg);
    srsran::test_delimit_logger delimiter("Testing data recovery. SN_SIZE={} COUNT={}", sn_size, tx_next);
    for (uint32_t count = tx_next; count < tx_next + n_sdus; ++count) {
      // Write SDU
      byte_buffer sdu = {sdu1};
      pdcp_tx->handle_sdu(std::move(sdu));
      pdcp_tx->handle_transmit_notification(pdcp_compute_sn(count, sn_size));

      // Get generated PDU
      ASSERT_EQ(test_frame.pdu_queue.size(), 1);
      exp_pdu_list.push(std::move(test_frame.pdu_queue.front()));
      test_frame.pdu_queue.pop();
      ASSERT_EQ(test_frame.pdu_queue.size(), 0);
    }

    pdcp_tx->data_recovery();

    // read the status report
    {
      pdcp_tx_pdu pdu = std::move(test_frame.pdu_queue.front());
      test_frame.pdu_queue.pop();
      byte_buffer exp_pdu = test_frame.compile_status_report();
      ASSERT_EQ(pdu.buf.length(), exp_pdu.length());
      ASSERT_EQ(pdu.buf, exp_pdu);
    }

    // read data PDUs
    for (uint32_t count = tx_next; count < tx_next + n_sdus; ++count) {
      pdcp_tx_pdu pdu = std::move(test_frame.pdu_queue.front());
      test_frame.pdu_queue.pop();
      pdcp_tx_pdu exp_pdu = std::move(exp_pdu_list.front());
      exp_pdu_list.pop();
      ASSERT_EQ(pdu.buf.length(), exp_pdu.buf.length());
      ASSERT_EQ(pdu.buf, exp_pdu.buf);
    }

    while (not test_frame.pdu_queue.empty()) {
      test_frame.pdu_queue.pop();
    }
  };

  if (sn_size == pdcp_sn_size::size12bits) {
    test_with_count(0);
    test_with_count(2048);
    test_with_count(4096);
  } else if (sn_size == pdcp_sn_size::size18bits) {
    test_with_count(0);
    test_with_count(131072);
    test_with_count(262144);
  } else {
    FAIL();
  }
}

///////////////////////////////////////////////////////////////////
// Finally, instantiate all testcases for each supported SN size //
///////////////////////////////////////////////////////////////////
std::string test_param_info_to_string(const ::testing::TestParamInfo<std::tuple<pdcp_sn_size, unsigned>>& info)
{
  fmt::memory_buffer buffer;
  fmt::format_to(buffer, "{}bit", pdcp_sn_size_to_uint(std::get<pdcp_sn_size>(info.param)));
  return fmt::to_string(buffer);
}

INSTANTIATE_TEST_SUITE_P(pdcp_tx_test_all_sn_sizes,
                         pdcp_tx_status_report_test,
                         ::testing::Combine(::testing::Values(pdcp_sn_size::size12bits, pdcp_sn_size::size18bits),
                                            ::testing::Values(1)),
                         test_param_info_to_string);

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
