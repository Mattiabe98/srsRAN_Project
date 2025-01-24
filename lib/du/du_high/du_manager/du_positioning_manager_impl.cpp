/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "du_positioning_manager_impl.h"
#include "procedures/positioning_procedures.h"

using namespace srsran;
using namespace srs_du;

du_positioning_manager_impl::du_positioning_manager_impl(du_cell_manager& cell_mng_, du_ue_manager& ue_mng_) :
  cell_mng(cell_mng_), ue_mng(ue_mng_)
{
}

du_trp_info_response du_positioning_manager_impl::request_trp_info()
{
  return prepare_trp_info();
}

async_task<du_positioning_info_response>
du_positioning_manager_impl::request_positioning_info(const du_positioning_info_request& req)
{
  return start_du_ue_positioning_info_procedure(req, cell_mng, ue_mng);
}

async_task<du_positioning_meas_response>
du_positioning_manager_impl::request_positioning_measurement(const du_positioning_meas_request& req)
{
  return start_positioning_measurement_procedure(req);
}
