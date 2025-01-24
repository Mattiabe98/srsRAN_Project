/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "common.h"
#include "positioning_ids.h"
#include "srsran/ran/cause/f1ap_cause.h"
#include "srsran/ran/crit_diagnostics.h"
#include "srsran/ran/nr_cgi.h"
#include <optional>
#include <variant>
#include <vector>

namespace srsran {

struct search_win_info_t {
  int16_t expected_propagation_delay;
  uint8_t delay_uncertainty;
};

struct expected_azimuth_aoa_t {
  uint16_t expected_azimuth_aoa_value;
  uint16_t expected_azimuth_aoa_uncertainty;
};

struct expected_zenith_aoa_t {
  uint16_t expected_zenith_aoa_value;
  uint16_t expected_zenith_aoa_uncertainty;
};

struct expected_ul_aoa_t {
  expected_azimuth_aoa_t               expected_azimuth_aoa;
  std::optional<expected_zenith_aoa_t> expected_zenith_aoa;
};

struct expected_zoa_only_t {
  expected_zenith_aoa_t expected_zoa_only;
};

using angle_meas_type_t = std::variant<expected_ul_aoa_t, expected_zoa_only_t>;

struct aoa_assist_info_t {
  angle_meas_type_t                       angle_meas;
  std::optional<lcs_to_gcs_translation_t> lcs_to_gcs_translation;
};

struct trp_meas_request_item_ext_ies_t {
  std::optional<nr_cell_global_id_t> cgi_nr;
  std::optional<aoa_assist_info_t>   aoa_search_win;
  std::optional<uint8_t>             nof_trp_rx_teg;
  std::optional<uint8_t>             nof_trp_rx_tx_teg;
};

struct trp_meas_request_item_t {
  trp_id_t                                       trp_id;
  std::optional<search_win_info_t>               search_win_info;
  std::optional<trp_meas_request_item_ext_ies_t> ie_exts;
};

enum class trp_meas_quantities_item_t {
  gnb_rx_tx_time_diff,
  ul_srs_rsrp,
  ul_ao_a,
  ul_rtoa,
  multiple_ul_ao_a,
  ul_srs_rsrp_p,
};

struct trp_meas_quantities_list_item_t {
  trp_meas_quantities_item_t trp_meas_quantities_item;
  std::optional<uint8_t>     timing_report_granularity_factor;
};

struct measurement_request_t {
  lmf_meas_id_t                                lmf_meas_id;
  std::vector<trp_meas_request_item_t>         trp_meas_request_list;
  report_characteristics_t                     report_characteristics;
  std::optional<meas_periodicity_t>            meas_periodicity;
  std::vector<trp_meas_quantities_list_item_t> trp_meas_quantities;

  // TODO: Add missing optional values.
};

enum class trp_measured_results_value_t { ul_angle_of_arrival, ul_srs_rsrp, ul_rtoa, gnb_rx_tx_time_diff };

struct time_stamp_slot_idx_t {
  enum class idx_type { scs15, scs30, scs60, scs120 };

  idx_type type;
  uint8_t  value;
};

struct time_stamp_t {
  uint16_t                sys_frame_num;
  time_stamp_slot_idx_t   slot_idx;
  std::optional<uint64_t> meas_time;
};

struct timing_meas_quality_t {
  enum class resolution_t { m0dot1, m1, m10, m30 };

  uint8_t      meas_quality;
  resolution_t resolution;
};

struct angle_meas_quality_t {
  enum class resolution_t { deg0dot1 };

  uint16_t                azimuth_quality;
  std::optional<uint16_t> zenith_quality;
  resolution_t            resolution;
};

using trp_meas_quality_t = std::variant<timing_meas_quality_t, angle_meas_quality_t>;

struct meas_beam_info_t {
  std::optional<uint8_t> prs_res_id;
  std::optional<uint8_t> prs_res_set_id;
  std::optional<uint8_t> ssb_idx;
};

struct trp_meas_result_item_t {
  trp_measured_results_value_t      measured_results_value;
  time_stamp_t                      time_stamp;
  std::optional<trp_meas_quality_t> meas_quality;
  std::optional<meas_beam_info_t>   meas_beam_info;
  // TODO: Add optional ext ies.
};

struct trp_measurement_response_item_t {
  trp_id_t                            trp_id;
  std::vector<trp_meas_result_item_t> meas_result;
  std::optional<nr_cell_global_id_t>  cgi_nr;
};

struct measurement_response_t {
  lmf_meas_id_t                                lmf_meas_id;
  ran_meas_id_t                                ran_meas_id;
  std::vector<trp_measurement_response_item_t> trp_meas_resp_list;
  std::optional<crit_diagnostics_t>            crit_diagnostics;
};

struct measurement_failure_t {
  lmf_meas_id_t                            lmf_meas_id;
  std::variant<f1ap_cause_t, f1ap_cause_t> cause;
  std::optional<crit_diagnostics_t>        crit_diagnostics;
};

struct measurement_report_t {
  lmf_meas_id_t                                lmf_meas_id;
  ran_meas_id_t                                ran_meas_id;
  std::vector<trp_measurement_response_item_t> trp_meas_report_list;
};

struct trp_meas_upd_item_ext_ies_container_t {
  std::optional<uint8_t> nof_trp_rx_teg;
  std::optional<uint8_t> nof_trp_rx_tx_teg;
};

struct trp_measurement_update_item_t {
  trp_id_t                                             trp_id;
  std::optional<aoa_assist_info_t>                     ao_a_win_info;
  std::optional<trp_meas_upd_item_ext_ies_container_t> ie_exts;
};

struct measurement_update_t {
  lmf_meas_id_t lmf_meas_id;
  ran_meas_id_t ran_meas_id;
  // Optional values:
  std::vector<trp_measurement_update_item_t> trp_meas_upd_list;
  // TODO: Add missing optional values.
};

struct measurement_abort_t {
  lmf_meas_id_t lmf_meas_id;
  ran_meas_id_t ran_meas_id;
};

} // namespace srsran
