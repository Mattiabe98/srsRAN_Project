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

#include "../support/prach_context_repository.h"

namespace srsran {
namespace ofh {

struct uplane_message_decoder_results;

/// \brief Open Fronthaul User-Plane PRACH symbol data flow writer.
///
/// Writes IQ data received in an Open Fronthaul message to the corresponding PRACH buffer.
class uplane_prach_symbol_data_flow_writer
{
public:
  uplane_prach_symbol_data_flow_writer(span<const unsigned>                      prach_eaxc_,
                                       srslog::basic_logger&                     logger_,
                                       std::shared_ptr<prach_context_repository> prach_context_repo_) :
    prach_eaxc(prach_eaxc_.begin(), prach_eaxc_.end()),
    logger(logger_),
    prach_context_repo_ptr(prach_context_repo_),
    prach_context_repo(*prach_context_repo_ptr)
  {
    srsran_assert(prach_context_repo_ptr, "Invalid PRACH context repository");
  }

  /// Writes the given decoder results in the corresponding PRACH buffer.
  void write_to_prach_buffer(unsigned eaxc, const uplane_message_decoder_results& results);

private:
  const static_vector<unsigned, MAX_NOF_SUPPORTED_EAXC> prach_eaxc;
  srslog::basic_logger&                                 logger;
  std::shared_ptr<prach_context_repository>             prach_context_repo_ptr;
  prach_context_repository&                             prach_context_repo;
};

} // namespace ofh
} // namespace srsran
