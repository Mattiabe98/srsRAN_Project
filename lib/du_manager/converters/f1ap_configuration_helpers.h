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

#include "srsran/adt/byte_buffer.h"
#include "srsran/du/du_cell_config.h"
#include "srsran/du_manager/du_manager_params.h"
#include "srsran/ran/ntn.h"

namespace srsran {

namespace srs_du {

/// \brief Derive packed cell MIB from DU cell configuration.
/// \param[in] du_cfg DU Cell Configuration.
/// \return byte buffer with packed cell MIB.
byte_buffer make_asn1_rrc_cell_mib_buffer(const du_cell_config& du_cfg);

/// \brief Derive packed cell SIB1 from DU cell configuration.
/// \param[in] du_cfg DU Cell Configuration.
/// \param[out] json_string String where the RRC ASN.1 SIB1 is stored in json format. If nullptr, no conversion takes
/// place.
/// \return byte buffer with packed cell SIB1.
byte_buffer make_asn1_rrc_cell_sib1_buffer(const du_cell_config& du_cfg, std::string* js_str = nullptr);

byte_buffer make_asn1_rrc_cell_sib19_buffer(const sib19_info& sib19_params, std::string* js_str = nullptr);

/// \brief Generate packed cell BCCH-DL-SCH messages (SIB1 + SI messages) from DU cell configuration.
/// \param[in] du_cfg DU Cell Configuration.
/// \return byte buffer with packed cell BCCH-DL-SCH message.
std::vector<byte_buffer> make_asn1_rrc_cell_bcch_dl_sch_msgs(const du_cell_config& du_cfg);

/// \brief Derive packed cell measurementTimingConfiguration from DU cell configuration.
/// \param[in] du_cfg DU Cell Configuration.
/// \return byte buffer with packed cell measurementTimingConfiguration as per TS38.331.
byte_buffer make_asn1_meas_time_cfg_buffer(const du_cell_config& du_cfg);

struct f1_setup_request_message;

/// \brief Generate request message for F1AP to initiate an F1 Setup Request procedure.
void fill_f1_setup_request(f1_setup_request_message&            req,
                           const du_manager_params::ran_params& ran_params,
                           std::vector<std::string>*            sib1_jsons = nullptr);

} // namespace srs_du

} // namespace srsran
