/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "srsran/support/resource_usage/resource_usage_utils.h"

using namespace srsran;
using namespace resource_usage_utils;

/// Converts rusage struct to the snapshot.
static cpu_snapshot to_snapshot(const ::rusage& rusg)
{
  cpu_snapshot s;
  s.tp          = rusage_meas_clock::now();
  s.user_time   = std::chrono::seconds{rusg.ru_utime.tv_sec} + std::chrono::microseconds{rusg.ru_utime.tv_usec};
  s.system_time = std::chrono::seconds{rusg.ru_stime.tv_sec} + std::chrono::microseconds{rusg.ru_stime.tv_usec};
  s.max_rss     = rusg.ru_maxrss;

  return s;
}

expected<cpu_snapshot, int> resource_usage_utils::cpu_usage_now(rusage_measurement_type type)
{
  ::rusage ret;
  if (::getrusage(static_cast<__rusage_who_t>(type), &ret) == 0) {
    return to_snapshot(ret);
  }
  return make_unexpected(errno);
}

measurements resource_usage_utils::operator+(const measurements& lhs, const measurements& rhs)
{
  measurements sum;
  sum.duration    = lhs.duration + lhs.duration;
  sum.user_time   = lhs.user_time + rhs.user_time;
  sum.system_time = lhs.system_time + lhs.system_time;
  sum.max_rss     = std::max(lhs.max_rss, rhs.max_rss);

  return sum;
}

resource_usage_metrics resource_usage_utils::res_usage_measurements_to_metrics(measurements              measurements,
                                                                               std::chrono::microseconds period)
{
  static constexpr unsigned BYTES_IN_KB = 1024;

  auto total_cpu_time_used = measurements.user_time + measurements.system_time;

  resource_usage_metrics metrics = {};
  metrics.cpu_stats.cpu_usage_percentage =
      static_cast<double>(total_cpu_time_used.count()) / static_cast<double>(period.count());
  metrics.cpu_stats.cpu_utilization_nof_cpus =
      static_cast<double>(total_cpu_time_used.count()) / static_cast<double>(measurements.duration.count());
  metrics.memory_stats.memory_usage = units::bytes(BYTES_IN_KB * measurements.max_rss);
  return metrics;
}
