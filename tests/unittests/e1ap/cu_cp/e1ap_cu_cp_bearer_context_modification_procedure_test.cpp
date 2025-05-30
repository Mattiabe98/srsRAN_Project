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

#include "e1ap_cu_cp_test_helpers.h"
#include "srsran/support/async/async_test_utils.h"
#include "srsran/support/test_utils.h"
#include <gtest/gtest.h>

using namespace srsran;
using namespace srs_cu_cp;
using namespace asn1::e1ap;

class e1ap_cu_cp_bearer_context_modification_test : public e1ap_cu_cp_test
{
protected:
  void start_procedure(const e1ap_bearer_context_modification_request& req)
  {
    run_bearer_context_setup(req.ue_index,
                             int_to_gnb_cu_up_ue_e1ap_id(test_rgen::uniform_int<uint64_t>(
                                 gnb_cu_up_ue_e1ap_id_to_uint(gnb_cu_up_ue_e1ap_id_t::min),
                                 gnb_cu_up_ue_e1ap_id_to_uint(gnb_cu_up_ue_e1ap_id_t::max) - 1)));

    t = e1ap->handle_bearer_context_modification_request(req);
    t_launcher.emplace(t);

    ASSERT_FALSE(t.ready());
    ASSERT_EQ(this->e1ap_pdu_notifier.last_e1ap_msg.pdu.init_msg().value.type().value,
              e1ap_elem_procs_o::init_msg_c::types::bearer_context_mod_request);
  }

  bool was_bearer_context_modification_request_sent(gnb_cu_cp_ue_e1ap_id_t cu_cp_ue_e1ap_id) const
  {
    if (this->e1ap_pdu_notifier.last_e1ap_msg.pdu.type().value != e1ap_pdu_c::types::init_msg) {
      return false;
    }
    if (this->e1ap_pdu_notifier.last_e1ap_msg.pdu.init_msg().value.type().value !=
        e1ap_elem_procs_o::init_msg_c::types_opts::bearer_context_mod_request) {
      return false;
    }
    auto& req = this->e1ap_pdu_notifier.last_e1ap_msg.pdu.init_msg().value.bearer_context_mod_request();

    return req->gnb_cu_cp_ue_e1ap_id == gnb_cu_cp_ue_e1ap_id_to_uint(cu_cp_ue_e1ap_id);
  }

  bool was_bearer_context_modification_successful() const
  {
    if (not t.ready()) {
      return false;
    }
    return t.get().success;
  }

  async_task<e1ap_bearer_context_modification_response>                        t;
  std::optional<lazy_task_launcher<e1ap_bearer_context_modification_response>> t_launcher;
};

/// Test the successful bearer context modification procedure (CU-CP initiated)
TEST_F(e1ap_cu_cp_bearer_context_modification_test, when_request_sent_then_procedure_waits_for_response)
{
  // Test Preamble.
  auto request = generate_bearer_context_modification_request(uint_to_ue_index(
      test_rgen::uniform_int<uint64_t>(ue_index_to_uint(ue_index_t::min), ue_index_to_uint(ue_index_t::max))));

  // Start BEARER CONTEXT MODIFICATION procedure.
  this->start_procedure(request);

  auto& ue = test_ues[request.ue_index];

  // The BEARER CONTEXT MODIFICATION was sent to CU-UP and CU-CP is waiting for response.
  ASSERT_TRUE(was_bearer_context_modification_request_sent(ue.cu_cp_ue_e1ap_id.value()));
  ASSERT_FALSE(t.ready());
}

TEST_F(e1ap_cu_cp_bearer_context_modification_test, when_response_received_then_procedure_successful)
{
  // Test Preamble.
  auto request = generate_bearer_context_modification_request(uint_to_ue_index(
      test_rgen::uniform_int<uint64_t>(ue_index_to_uint(ue_index_t::min), ue_index_to_uint(ue_index_t::max))));

  // Start BEARER CONTEXT MODIFICATION procedure and return back the response from the CU-UP.
  this->start_procedure(request);

  auto&        ue = test_ues[request.ue_index];
  e1ap_message response =
      generate_bearer_context_modification_response(ue.cu_cp_ue_e1ap_id.value(), ue.cu_up_ue_e1ap_id.value());
  e1ap->handle_message(response);

  // The BEARER CONTEXT MODIFICATION RESPONSE was received and the CU-CP completed the procedure.
  ASSERT_TRUE(was_bearer_context_modification_successful());
}

TEST_F(e1ap_cu_cp_bearer_context_modification_test, when_ue_setup_failure_received_then_procedure_unsuccessful)
{
  // Test Preamble.
  auto request = generate_bearer_context_modification_request(uint_to_ue_index(
      test_rgen::uniform_int<uint64_t>(ue_index_to_uint(ue_index_t::min), ue_index_to_uint(ue_index_t::max))));

  // Start BEARER CONTEXT MODIFICATION procedure and return back the failure response from the CU-UP.
  this->start_procedure(request);

  auto& ue = test_ues[request.ue_index];

  e1ap_message response =
      generate_bearer_context_modification_failure(ue.cu_cp_ue_e1ap_id.value(), ue.cu_up_ue_e1ap_id.value());
  e1ap->handle_message(response);

  // The BEARER CONTEXT MODIFICATION FAILURE was received and the CU-CP completed the procedure with failure.
  ASSERT_FALSE(was_bearer_context_modification_successful());
}
