/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

namespace srsran {

namespace band_helper {

const unsigned KHZ_TO_HZ = 1000U;
const double   HZ_TO_KHZ = 1e-3;

/// Offset \f$F_{REF-Offs}\f$ expressed in Hz, for 3GHz-24.25GHz freq. range, as per Table 5.4.2.1-1, TS 38.104.
const double N_REF_OFFSET_3_GHZ_24_5_GHZ = 3e9;
/// Offset \f$F_{REF-Offs}\f$ expressed in Hz, for 24.25GHz-100GHz freq. range, as per Table 5.4.2.1-1, TS 38.104.
const double N_REF_OFFSET_24_5_GHZ_100_GHZ = 24.25e9;
/// Offset \f$F_{REF-Offs}\f$ expressed in Hz, for 24.25GHz-100GHz freq. range, as per Table 5.4.2.1-1, TS 38.104.
const double BASE_FREQ_GSCN_RASTER_24_5_GHZ_100_GHZ = 24250.08e6;
/// Maximum frequency that can be converted to ARFCN, as per Table 5.4.2.1-1, TS 38.104.
const double MAX_RASTER_FREQ = 100e9;
/// Min ARFCN value for range of frequencies 3GHz-24.25GHz, as per Table 5.4.2.1-1, TS 38.104.
const unsigned MIN_ARFCN_3_GHZ_24_5_GHZ = 600000;
/// Min ARFCN value for range of frequencies above 24.25GHz, as per Table 5.4.2.1-1, TS 38.104.
const unsigned MIN_ARFCN_24_5_GHZ_100_GHZ = 2016667;
/// Freq. step in Hz, corresponding to \f$N\f$ for 0GHz-3GHz freq. range, as per Table 5.4.3.1-1, TS 38.104.
const double N_SIZE_SYNC_RASTER_1_HZ = 1200e3;
/// Freq. step in Hz, corresponding to \f$M\f$ for 0GHz-3GHz freq. range, as per Table 5.4.3.1-1, TS 38.104.
const double M_SIZE_SYNC_RASTER_1_HZ = 50e3;
/// Freq. step in Hz, corresponding to \f$M\f$ for 3GHz-24.25GHz freq. range, as per Table 5.4.3.1-1, TS 38.104.
const double N_SIZE_SYNC_RASTER_2_HZ = 1440e3;
/// Freq. step in Hz, corresponding to \f$M\f$ for 24.25GHz-100GHz freq. range, as per Table 5.4.3.1-1, TS 38.104.
const double N_SIZE_SYNC_RASTER_3_HZ = 17280e3;
/// Upper-bound for \f$N\f$ within 0GHz-3GHz freq. range, as per Table 5.4.3.1-1, TS 38.104.
const unsigned N_UB_SYNC_RASTER_1 = 2500;
/// Upper-bound for \f$N\f$ within 3GHz-24.25GHz freq. range, as per Table 5.4.3.1-1, TS 38.104.
const unsigned N_UB_SYNC_RASTER_2 = 14757;
/// Minimum GSCN value for the freq range 0GHz - 3GHz, as per Table 5.4.3.1-1, TS 38.104.
const unsigned MIN_GSCN_FREQ_0_3GHZ = 2;
/// Minimum GSCN value for the freq range 3GHz - 24.5GHz, as per Table 5.4.3.1-1, TS 38.104.
const unsigned MIN_GSCN_FREQ_3GHZ_24_5GHZ = 7499;
/// Minimum GSCN value for the freq range 24.5GHz - 100GHz, as per Table 5.4.3.1-1, TS 38.104.
const unsigned MIN_GSCN_FREQ_24_5GHZ_100GHZ = 22256;
/// Maximum GSCN value for the freq range 24.5GHz - 100GHz, as per Table 5.4.3.1-1, TS 38.104.
const unsigned MAX_GSCN_FREQ_24_5GHZ_100GHZ = 26639;
/// Lower-bound for GSCN within 3GHz-24.25GHz freq. range, as per Table 5.4.3.1-1, TS 38.104.
const unsigned GSCN_LB_SYNC_RASTER_2 = 7499U;
/// Lower-bound for GSCN for band n79 and bandwidth equal to or greater than 40MHz, as per Table 5.4.3.3-1, TS 38.104.
const unsigned GSCN_LB_N_79_BW_40_MHZ = 8480U;
/// Lower-bound for GSCN for band n104, as per Table 5.4.3.3-1, TS 38.104.
const unsigned GSCN_LB_N_104 = 9882U;

} // namespace band_helper
} // namespace srsran
