/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * This file is part of srsRAN.
 *
 * srsRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#include "../../../../lib/ofh/compression/packing_utils_generic.h"
#include "../../../../lib/ofh/compression/quantizer.h"
#include "ofh_compression_test_data.h"
#include "srsran/ofh/compression/compression_factory.h"
#include "srsran/ofh/compression/compression_properties.h"
#include "srsran/srslog/srslog.h"
#include "srsran/support/units.h"
#include <gtest/gtest.h>

using namespace srsran;
using namespace ofh;

// Output stream formatting to make GTest results human-readable.
namespace srsran {
std::ostream& operator<<(std::ostream& os, const test_case_t& test_case)
{
  return os << fmt::format("prbs={}, compression={}, iq_width={}, scaling={}",
                           test_case.nof_prb,
                           to_string(test_case.type),
                           test_case.cIQ_width,
                           test_case.iq_scaling);
}
} // namespace srsran

namespace {

using OFHCompressionParams = std::tuple<std::string, test_case_t>;

class OFHCompressionFixture : public ::testing::TestWithParam<OFHCompressionParams>
{
protected:
  void SetUp() override
  {
    test_case_t test_case = std::get<1>(GetParam());
    params.type           = test_case.type;
    params.data_width     = test_case.cIQ_width;

    compressor = create_iq_compressor(params.type, logger, test_case.iq_scaling, std::get<0>(GetParam()));
    ASSERT_NE(compressor, nullptr);
    decompressor = create_iq_decompressor(params.type, logger, std::get<0>(GetParam()));
    ASSERT_NE(decompressor, nullptr);
  }

  ru_compression_params            params       = {};
  std::unique_ptr<iq_compressor>   compressor   = nullptr;
  std::unique_ptr<iq_decompressor> decompressor = nullptr;
  srslog::basic_logger&            logger       = srslog::fetch_basic_logger("TEST", false);
};

// Verify data after compression and following decompression.
TEST_P(OFHCompressionFixture, match_test_case_result_and_decompress_to_original)
{
  // Instantiate quantizer object.
  quantizer q(params.data_width);

  // Read test data.
  test_case_t          test_case        = std::get<1>(GetParam());
  float                iq_scaling       = test_case.iq_scaling;
  unsigned             nof_prb          = test_case.nof_prb;
  std::vector<cf_t>    test_data        = test_case.symbols.read();
  std::vector<int16_t> test_compr_data  = test_case.compressed_IQs.read();
  std::vector<uint8_t> test_compr_param = test_case.compressed_params.read();

  std::vector<cbf16_t> test_data_cbf16(test_data.size());
  srsvec::convert(test_data_cbf16, test_data);

  unsigned prb_size = get_compressed_prb_size(params).value();

  // Prepare vectors to store compression/decompression results.
  std::vector<uint8_t> compressed_data(nof_prb * prb_size);
  std::vector<cbf16_t> decompressed_data(nof_prb * NOF_SUBCARRIERS_PER_RB);

  // Compress input test data.
  compressor->compress(compressed_data, test_data_cbf16, params);

  // Verify compressed IQs.
  for (unsigned j = 0; j != nof_prb; ++j) {
    unsigned prb_iq_data_offset = 0;
    if (params.type == srsran::ofh::compression_type::BFP) {
      // Parameter should match, it is serialized in the first byte.
      ASSERT_EQ(compressed_data[j * prb_size], test_compr_param[j]) << fmt::format("wrong PRB={} param", j);
      prb_iq_data_offset = 1;
    }
    // Data should match.
    bit_buffer_reader buffer = bit_buffer_reader::from_bytes(
        {&compressed_data[j * prb_size + prb_iq_data_offset], prb_size - prb_iq_data_offset});

    for (unsigned i = 0, read_pos = 0; i != NOF_SUBCARRIERS_PER_RB * 2; ++i) {
      int16_t sample = q.sign_extend(unpack_bits(buffer, read_pos, params.data_width));
      read_pos += params.data_width;

      constexpr uint16_t err_tolerance = 1;

      ASSERT_TRUE(std::abs(sample - test_compr_data[j * NOF_SUBCARRIERS_PER_RB * 2 + i]) <= err_tolerance)
          << fmt::format("Compressed samples mismatch at position {}", j * NOF_SUBCARRIERS_PER_RB * 2 + i);
    }
  }

  // Decompress back.
  decompressor->decompress(decompressed_data, compressed_data, params);

  // Check resulting samples versus the original ones considering also a precision loss caused by initial
  // single-precision float to brain float conversion.
  float resolution = 2.0F / (1 << (params.data_width - 1)) + 1 / 256.0F;

  std::vector<cf_t> decompressed_data_cf(nof_prb * NOF_SUBCARRIERS_PER_RB);
  srsvec::convert(decompressed_data_cf, decompressed_data);
  for (auto& result : decompressed_data_cf) {
    result = {std::real(result) / iq_scaling, std::imag(result) / iq_scaling};
  }

  ASSERT_TRUE(std::equal(test_data.begin(),
                         test_data.end(),
                         decompressed_data_cf.begin(),
                         [&](cf_t& test_value, cf_t& result) {
                           // Make sure the sign is the same and diff is minimal.
                           return (std::abs(std::real(test_value) - std::real(result)) <= resolution) &&
                                  (std::abs(std::imag(test_value) - std::imag(result)) <= resolution);
                         }))
      << "Decompressed data don't match the original ones";
}

// Verify zero input is correctly processed.
TEST_P(OFHCompressionFixture, zero_input_compression_is_correct)
{
  // Generate array of random complex float samples.
  unsigned          nof_prb = std::get<1>(GetParam()).nof_prb;
  std::vector<cf_t> test_data(nof_prb * NOF_SUBCARRIERS_PER_RB);
  std::fill(test_data.begin(), test_data.end(), 0);

  std::vector<cbf16_t> test_data_cbf16(test_data.size());
  srsvec::convert(test_data_cbf16, test_data);

  unsigned prb_size = get_compressed_prb_size(params).value();

  std::vector<uint8_t> compressed_data(nof_prb * prb_size);
  std::vector<cbf16_t> decompressed_data(nof_prb * NOF_SUBCARRIERS_PER_RB);

  // Compress it.
  compressor->compress(compressed_data, test_data_cbf16, params);
  // Decompress back.
  decompressor->decompress(decompressed_data, compressed_data, params);

  std::vector<cf_t> decompressed_data_cf(nof_prb * NOF_SUBCARRIERS_PER_RB);
  srsvec::convert(decompressed_data_cf, decompressed_data);
  ASSERT_EQ(test_data, decompressed_data_cf);
}

// Verify BPSK modulated data are correctly processed.
TEST(ru_compression_test, bpsk_input_compression_is_correct)
{
  srslog::basic_logger& logger = srslog::fetch_basic_logger("TEST", false);

  // Instantiate quantizer object.
  quantizer q(9);

  std::vector<cf_t> test_data = {0, 0, 0, 0,  0, 0, 0,  0, 0, 1,  1,  1, -1, -1, 1, 1,  -1, 1,  1,  1, -1, 1,  1,  1,
                                 1, 1, 1, -1, 1, 1, -1, 1, 1, -1, -1, 1, -1, 1,  1, -1, -1, -1, -1, 1, -1, -1, -1, 1};
  // Reference compressed data taken from Matlab.
  std::vector<unsigned>          test_compr_param     = {7, 7, 7, 7};
  std::vector<std::vector<cf_t>> test_compressed_prbs = {
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255},
      {-255, -255, 255, 255, -255, 255, 255, 255, -255, 255, 255, 255},
      {255, 255, 255, -255, 255, 255, -255, 255, 255, -255, -255, 255},
      {-255, 255, 255, -255, -255, -255, -255, 255, -255, -255, -255, 255}};

  std::unique_ptr<iq_compressor> compressor = create_iq_compressor(compression_type::BFP, logger, 1.0, "generic");
  ru_compression_params          params     = {compression_type::BFP, 9};
  unsigned                       prb_size   = get_compressed_prb_size(params).value();
  std::vector<uint8_t>           compressed_data(4 * prb_size);

  // Compress it.
  std::vector<cbf16_t> test_data_cbf16(test_data.size());
  srsvec::convert(test_data_cbf16, test_data);
  compressor->compress(compressed_data, test_data_cbf16, params);

  for (unsigned j = 0; j != 4; ++j) {
    // Parameter should match.
    ASSERT_EQ(compressed_data[j * prb_size], test_compr_param[j]) << fmt::format("wrong PRB={} param", j);

    // Skip first byte storing the compression parameter.
    bit_buffer_reader buffer = bit_buffer_reader::from_bytes({&compressed_data[j * prb_size + 1], prb_size - 1});

    // Data should match.
    for (unsigned i = 0, read_pos = 0; i != NOF_SUBCARRIERS_PER_RB; ++i) {
      int16_t re = q.sign_extend(unpack_bits(buffer, read_pos, 9));
      read_pos += params.data_width;
      int16_t im = q.sign_extend(unpack_bits(buffer, read_pos, 9));
      read_pos += params.data_width;

      ASSERT_TRUE(std::abs(re - std::real(test_compressed_prbs[j][i])) <= 1)
          << fmt::format("mismatch at position [{}][{}]", j, i);
      ASSERT_TRUE(std::abs(im - std::imag(test_compressed_prbs[j][i])) <= 1)
          << fmt::format("mismatch at position [{}][{}]", j, i);
    }
  }
}

INSTANTIATE_TEST_SUITE_P(OFHCompressionTestSuite,
                         OFHCompressionFixture,
                         ::testing::Combine(::testing::Values("generic"
#ifdef __AVX2__
                                                              ,
                                                              "avx2"
#endif // __AVX2__
#ifdef __AVX512F__
                                                              ,
                                                              "avx512"
#endif // __AVX512F__
#ifdef __ARM_NEON
                                                              ,
                                                              "neon"
#endif // __ARM_NEON
                                                              ),
                                            ::testing::ValuesIn(ofh_compression_test_data)));

} // namespace
