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

#include "srsran/ran/slot_point.h"
#include <condition_variable>
#include <mutex>

namespace srsran {

/// This object creates a barrier for multiple threads when wait() is called. Once the count reaches zero,
/// a callback is invoked.
class slot_sync_point
{
public:
  slot_sync_point()                                  = default;
  slot_sync_point(const slot_sync_point&)            = delete;
  slot_sync_point(slot_sync_point&&)                 = delete;
  slot_sync_point& operator=(const slot_sync_point&) = delete;
  slot_sync_point& operator=(slot_sync_point&&)      = delete;

  /// \brief Called when thread enters synchronization point. Steps:
  /// 1. If the current thread is the first to call wait(), the count is set to the number of workers.
  /// 2. The count is decremented.
  ///    - If count > 0, the current thread blocks waiting for count to reach zero.
  ///    - If count==0, the current thread invokes completion callback, and notifies remaining threads to unlock.
  /// \param sl current slot.
  /// \param nof_workers number of workers currently passing through synchronization point.
  /// \param func Completion callback invoked when count reaches zero.
  template <typename CompletionFunction>
  void wait(slot_point sl, size_t nof_workers, const CompletionFunction& func)
  {
    std::unique_lock<std::mutex> lock(mutex);
    srsran_sanity_check(nof_workers > 0, "Invalid count");
    if (sl != last_sl) {
      // Initialize barrier.
      count   = nof_workers;
      last_sl = sl;
    }

    if (--count == 0) {
      // Phase completion step.
      func();
      lock.unlock();
      cvar.notify_all();
    } else {
      while (count > 0) {
        cvar.wait(lock);
      }
    }
  }

private:
  slot_point              last_sl;
  size_t                  count;
  std::mutex              mutex;
  std::condition_variable cvar;
};

} // namespace srsran
