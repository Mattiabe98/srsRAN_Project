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

#include <cstdint>

namespace srsran {

/// \brief NZP-CSI-RS-ResourceId is used to identify one NZP-CSI-RS-Resource.
/// \remark See TS 38.331, \c NZP-CSI-RS-ResourceId and \c maxNrofNZP-CSI-RS-Resources.
enum nzp_csi_rs_res_id_t : uint8_t {
  MIN_NZP_CSI_RS_RESOURCE_ID   = 0,
  MAX_NZP_CSI_RS_RESOURCE_ID   = 191,
  MAX_NOF_NZP_CSI_RS_RESOURCES = 192
};

/// \brief NZP-CSI-RS-ResourceSetId is used to identify one NZP-CSI-RS-ResourceSet.
/// \remark See TS 38.331, \c NZP-CSI-RS-ResourceSetId.
enum nzp_csi_rs_res_set_id_t : uint8_t {
  MIN_NZP_CSI_RS_RESOURCE_SET_ID       = 0,
  MAX_NZP_CSI_RS_RESOURCE_SET_ID       = 63,
  MAX_NOF_NZP_CSI_RS_RESOURCE_SETS     = 64,
  MAX_NOF_NZP_CSI_RS_RESOURCES_PER_SET = 16
};

/// \brief CSI-IM-ResourceId is used to identify one CSI-IM-Resource.
/// \remark See TS 38.331, \c CSI-IM-ResourceId.
enum csi_im_res_id_t : uint8_t {
  MIN_CSI_IM_RESOURCE_ID   = 0,
  MAX_CSI_IM_RESOURCE_ID   = 31,
  MAX_NOF_CSI_IM_RESOURCES = 32
};

/// \brief CSI-IM-ResourceSetId is used to identify CSI-IM-ResourceSets.
/// \remark See TS 38.331, \c CSI-IM-ResourceSetId.
enum csi_im_res_set_id_t : uint8_t {
  MIN_CSI_IM_RESOURCE_SET_ID       = 0,
  MAX_CSI_IM_RESOURCE_SET_ID       = 63,
  MAX_NOF_CSI_IM_RESOURCE_SETS     = 64,
  MAX_NOF_CSI_IM_RESOURCES_PER_SET = 8,
};

/// \brief CSI-SSB-ResourceSetId is used to identify one SS/PBCH block resource set.
/// \remark See TS 38.331, \c CSI-SSB-ResourceSetId.
enum csi_ssb_res_set_id_t {
  MIN_CSI_SSB_RESOURCE_SET_ID       = 0,
  MAX_CSI_SSB_RESOURCE_SET_ID       = 63,
  MAX_NOF_CSI_SSB_RESOURCE_SETS     = 64,
  MAX_NOF_CSI_SSB_RESOURCES_PER_SET = 64,
};

/// \brief ZP-CSI-ResourceId is used to identify one ZP-CSI-Resource.
/// \remark See TS 38.331, \c ZP-CSI-RS-ResourceId.
enum zp_csi_rs_res_id_t : uint8_t {
  MIN_ZP_CSI_RS_RESOURCE_ID   = 0,
  MAX_ZP_CSI_RS_RESOURCE_ID   = 31,
  MAX_NOF_ZP_CSI_RS_RESOURCES = 32
};

/// \brief ZP-CSI-ResourceSetId is used to identify one ZP-CSI-ResourceSet.
/// \remark See TS 38.331, \c ZP-CSI-RS-ResourceSetId.
enum zp_csi_rs_res_set_id_t : uint8_t {
  MIN_ZP_CSI_RS_RESOURCE_SET_ID   = 0,
  MAX_ZP_CSI_RS_RESOURCE_SET_ID   = 15,
  MAX_NOF_ZP_CSI_RS_RESOURCE_SETS = 16,
};

} // namespace srsran
