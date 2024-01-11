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
#include <limits>
#include <type_traits>

namespace srsran {

/// \brief GNB-CU-UE-F1AP-ID used to identify the UE in the F1AP-CU.
/// \remark See TS 38.473 Section 9.3.1.4: GNB-CU-UE-F1AP-ID valid values: (0..2^32-1)
constexpr static uint64_t MAX_NOF_CU_F1AP_UES = ((uint64_t)1 << 32);
enum class gnb_cu_ue_f1ap_id_t : uint64_t { min = 0, max = MAX_NOF_CU_F1AP_UES - 1, invalid = 0x1ffffffff };

constexpr inline uint64_t gnb_cu_ue_f1ap_id_to_uint(gnb_cu_ue_f1ap_id_t id)
{
  return static_cast<uint64_t>(id);
}

/// Convert integer to GNB-CU-UE-F1AP-ID type.
constexpr inline gnb_cu_ue_f1ap_id_t int_to_gnb_cu_ue_f1ap_id(uint64_t idx)
{
  return static_cast<gnb_cu_ue_f1ap_id_t>(idx);
}

/// \brief GNB-DU-UE-F1AP-ID used to identify the UE in the F1AP-DU.
/// \remark See TS 38.473 Section 9.3.1.5: GNB-DU-UE-F1AP-ID valid values: (0..2^32-1)
constexpr static uint64_t MAX_NOF_DU_F1AP_UES = (static_cast<uint64_t>(1) << static_cast<uint64_t>(32U));
enum class gnb_du_ue_f1ap_id_t : uint64_t { min = 0, max = MAX_NOF_DU_F1AP_UES - 1, invalid = 0x1ffffffff };

constexpr inline uint64_t gnb_du_ue_f1ap_id_to_uint(gnb_du_ue_f1ap_id_t id)
{
  return static_cast<uint64_t>(id);
}

/// Convert integer to GNB-DU-UE-F1AP-ID type.
constexpr inline gnb_du_ue_f1ap_id_t int_to_gnb_du_ue_f1ap_id(uint64_t idx)
{
  return static_cast<gnb_du_ue_f1ap_id_t>(idx);
}

} // namespace srsran
