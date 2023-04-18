/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "mac_test_mode_adapter.h"

using namespace srsran;

static const unsigned TEST_UE_DL_BUFFER_STATE_UPDATE_SIZE = 100000;

class test_ue_mac_sdu_tx_builder_adapter : public mac_sdu_tx_builder
{
  static constexpr unsigned TX_SDU_MAX_SIZE = 5000;

public:
  test_ue_mac_sdu_tx_builder_adapter() { tx_sdu.append(std::vector<uint8_t>(TX_SDU_MAX_SIZE, 0)); }

  byte_buffer_slice_chain on_new_tx_sdu(unsigned nof_bytes) override
  {
    return {byte_buffer_slice(tx_sdu, 0, std::min(nof_bytes, (unsigned)TX_SDU_MAX_SIZE))};
  }
  unsigned on_buffer_state_update() override { return TEST_UE_DL_BUFFER_STATE_UPDATE_SIZE; }

private:
  byte_buffer tx_sdu = {};
};

class test_ue_info_handler : public mac_ue_control_information_handler
{
public:
  test_ue_info_handler(const srs_du::du_test_config::test_ue_config& test_ue_cfg_,
                       const du_ue_index_t&                          test_ue_index_,
                       mac_ue_control_information_handler&           adapted_) :
    test_ue_cfg(test_ue_cfg_), test_ue_index(test_ue_index_), adapted(adapted_)
  {
  }

  void handle_dl_buffer_state_update_required(const mac_dl_buffer_state_indication_message& dl_bs) override
  {
    mac_dl_buffer_state_indication_message dl_bs_copy = dl_bs;
    if (test_ue_index == dl_bs.ue_index and test_ue_cfg.pdsch_active) {
      // It is the test UE. Set a positive DL buffer state if PDSCH is set to "activated".
      dl_bs_copy.bs = TEST_UE_DL_BUFFER_STATE_UPDATE_SIZE;
    }

    adapted.handle_dl_buffer_state_update_required(dl_bs_copy);
  }

private:
  const srs_du::du_test_config::test_ue_config& test_ue_cfg;
  const du_ue_index_t&                          test_ue_index;
  mac_ue_control_information_handler&           adapted;
};

class test_ue_configurator : public mac_ue_configurator
{
public:
  explicit test_ue_configurator(const srs_du::du_test_config::test_ue_config& test_ue_cfg_,
                                du_ue_index_t&                                test_ue_index_,
                                mac_ue_configurator&                          cfg_adaptee_,
                                mac_ue_control_information_handler&           ue_info_handler_) :
    test_ue_cfg(test_ue_cfg_),
    test_ue_index(test_ue_index_),
    ue_cfg_adapted(cfg_adaptee_),
    ue_info_handler(ue_info_handler_)
  {
  }

  async_task<mac_ue_create_response_message> handle_ue_create_request(const mac_ue_create_request_message& cfg) override
  {
    mac_ue_create_request_message cfg_adapted = cfg;
    if (cfg_adapted.crnti == test_ue_cfg.rnti) {
      // It is the test UE.

      // Save UE index.
      test_ue_index = cfg_adapted.ue_index;

      // Add adapters to the UE config bearers before passing it to MAC.
      cfg_adapted.bearers = adapt_bearers(cfg.bearers);
    }

    return launch_async([this, cfg_adapted](coro_context<async_task<mac_ue_create_response_message>>& ctx) mutable {
      CORO_BEGIN(ctx);

      // Create the UE in mac instance.
      CORO_AWAIT_VALUE(mac_ue_create_response_message ret, ue_cfg_adapted.handle_ue_create_request(cfg_adapted));

      if (test_ue_cfg.rnti == cfg_adapted.crnti) {
        // It is the test UE.

        if (test_ue_cfg.pdsch_active) {
          // Update DL buffer state automatically.
          ue_info_handler.handle_dl_buffer_state_update_required(
              {ret.ue_index, lcid_t::LCID_SRB1, TEST_UE_DL_BUFFER_STATE_UPDATE_SIZE});
        }
      }
      CORO_RETURN(ret);
    });
  }

  async_task<mac_ue_reconfiguration_response_message>
  handle_ue_reconfiguration_request(const mac_ue_reconfiguration_request_message& cfg) override
  {
    if (cfg.crnti == test_ue_cfg.rnti) {
      // If it is the test UE.
      auto cfg_adapted = cfg;

      // Add adapters to the UE config bearers before passing it to MAC.
      cfg_adapted.bearers_to_addmod = adapt_bearers(cfg.bearers_to_addmod);

      return ue_cfg_adapted.handle_ue_reconfiguration_request(cfg_adapted);
    }

    // otherwise, just forward config.
    return ue_cfg_adapted.handle_ue_reconfiguration_request(cfg);
  }

  async_task<mac_ue_delete_response_message> handle_ue_delete_request(const mac_ue_delete_request_message& cfg) override
  {
    return ue_cfg_adapted.handle_ue_delete_request(cfg);
  }

  void handle_ul_ccch_msg(du_ue_index_t ue_index, byte_buffer pdu) override
  {
    ue_cfg_adapted.handle_ul_ccch_msg(ue_index, std::move(pdu));
  }

private:
  std::vector<mac_logical_channel_config> adapt_bearers(const std::vector<mac_logical_channel_config>& orig_bearers)
  {
    static test_ue_mac_sdu_tx_builder_adapter dl_adapter;

    auto test_ue_adapted_bearers = orig_bearers;
    for (auto& bearer : test_ue_adapted_bearers) {
      if (bearer.lcid != lcid_t::LCID_SRB0) {
        if (test_ue_cfg.pdsch_active) {
          bearer.dl_bearer = &dl_adapter;
        }
      }
    }

    return test_ue_adapted_bearers;
  }

  const srs_du::du_test_config::test_ue_config& test_ue_cfg;
  du_ue_index_t&                                test_ue_index;
  mac_ue_configurator&                          ue_cfg_adapted;
  mac_ue_control_information_handler&           ue_info_handler;
};

class test_ue_cell_info_handler : public mac_cell_control_information_handler
{
public:
  test_ue_cell_info_handler(const srs_du::du_test_config::test_ue_config& test_ue_cfg_,
                            mac_cell_control_information_handler&         adaptee_) :
    test_ue_cfg(test_ue_cfg_), adapted(adaptee_)
  {
  }

  void handle_crc(const mac_crc_indication_message& msg) override
  {
    mac_crc_indication_message msg_copy = msg;
    for (auto& crc : msg_copy.crcs) {
      if (crc.rnti != test_ue_cfg.rnti) {
        // It is not the test UE.
        continue;
      }

      // Force CRC=OK for test UE.
      crc.tb_crc_success = true;
    }

    return adapted.handle_crc(msg_copy);
  }

  void handle_uci(const mac_uci_indication_message& msg) override
  {
    mac_uci_indication_message msg_copy = msg;
    for (auto& uci : msg_copy.ucis) {
      if (uci.rnti != test_ue_cfg.rnti) {
        // It is not the test UE.
        continue;
      }

      // In case of test UE, set HARQ-Info always equal to ACK.
      if (variant_holds_alternative<mac_uci_pdu::pucch_f0_or_f1_type>(uci.pdu)) {
        auto& f0 = variant_get<mac_uci_pdu::pucch_f0_or_f1_type>(uci.pdu);
        if (f0.harq_info.has_value()) {
          for (uci_pucch_f0_or_f1_harq_values& harq : f0.harq_info->harqs) {
            harq = uci_pucch_f0_or_f1_harq_values::ack;
          }
        }
      } else if (variant_holds_alternative<mac_uci_pdu::pusch_type>(uci.pdu)) {
        auto& pusch = variant_get<mac_uci_pdu::pusch_type>(uci.pdu);
        if (pusch.harq_info.has_value() and
            pusch.harq_info->harq_status == uci_pusch_or_pucch_f2_3_4_detection_status::crc_pass) {
          pusch.harq_info->payload.fill(0, pusch.harq_info->payload.size(), true);
        }
      } else if (variant_holds_alternative<mac_uci_pdu::pucch_f2_or_f3_or_f4_type>(uci.pdu)) {
        auto& f2 = variant_get<mac_uci_pdu::pucch_f2_or_f3_or_f4_type>(uci.pdu);
        if (f2.harq_info.has_value() and
            f2.harq_info->harq_status == uci_pusch_or_pucch_f2_3_4_detection_status::crc_pass) {
          f2.harq_info->payload.fill(0, f2.harq_info->payload.size(), true);
        }
      }
    }

    adapted.handle_uci(msg_copy);
  }

private:
  const srs_du::du_test_config::test_ue_config& test_ue_cfg;
  mac_cell_control_information_handler&         adapted;
};

mac_test_mode_adapter::mac_test_mode_adapter(std::unique_ptr<mac_interface>                mac_ptr_,
                                             const srs_du::du_test_config::test_ue_config& test_ue_cfg) :
  test_ue(test_ue_cfg), mac_ptr(std::move(mac_ptr_))
{
  ue_control_info_handler =
      std::make_unique<test_ue_info_handler>(test_ue, test_ue_index, mac_ptr->get_ue_control_info_handler());
  ue_configurator = std::make_unique<test_ue_configurator>(
      test_ue, test_ue_index, mac_ptr->get_ue_configurator(), *ue_control_info_handler);
}

mac_test_mode_adapter::~mac_test_mode_adapter() = default;

mac_cell_control_information_handler& mac_test_mode_adapter::get_control_info_handler(du_cell_index_t cell_index)
{
  if (cell_info_handler.size() <= cell_index) {
    cell_info_handler.resize(cell_index + 1);
  }
  if (cell_info_handler[cell_index] == nullptr) {
    cell_info_handler[cell_index] =
        std::make_unique<test_ue_cell_info_handler>(test_ue, mac_ptr->get_control_info_handler(cell_index));
  }
  return *cell_info_handler[cell_index];
}
