/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "srsran/phy/generic_functions/precoding/precoding_factories.h"
#include "channel_precoder_generic.h"
#include "srsran/support/cpu_features.h"

#ifdef __x86_64__
#include "channel_precoder_avx2.h"
#include "channel_precoder_avx512.h"
#endif // __x86_64__

using namespace srsran;

namespace {

class channel_precoder_factory_impl : public channel_precoder_factory
{
public:
  explicit channel_precoder_factory_impl(std::string precoder_type_) : precoder_type(std::move(precoder_type_)) {}

  std::unique_ptr<channel_precoder> create() override
  {
#ifdef __x86_64__
    bool supports_avx512 = cpu_supports_feature(cpu_feature::avx512f) && cpu_supports_feature(cpu_feature::avx512bw);
    bool supports_avx2   = cpu_supports_feature(cpu_feature::avx2);
    bool supports_fma    = cpu_supports_feature(cpu_feature::fma);
    if (((precoder_type == "avx512") || (precoder_type == "auto")) && supports_avx512) {
      return std::make_unique<channel_precoder_avx512>();
    }
    if (((precoder_type == "avx2") || (precoder_type == "auto")) && supports_avx2 && supports_fma) {
      return std::make_unique<channel_precoder_avx2>();
    }
#endif // __x86_64__
    if ((precoder_type == "generic") || (precoder_type == "auto")) {
      return std::make_unique<channel_precoder_generic>();
    }
    // Do not instantiate the channel precoder if the hardware does not support the implementation.
    return nullptr;
  }

private:
  std::string precoder_type;
};

} // namespace

std::shared_ptr<channel_precoder_factory> srsran::create_channel_precoder_factory(const std::string& precoder_type)
{
  return std::make_shared<channel_precoder_factory_impl>(precoder_type);
}