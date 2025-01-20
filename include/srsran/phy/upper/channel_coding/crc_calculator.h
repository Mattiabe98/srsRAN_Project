/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/adt/bit_buffer.h"
#include "srsran/adt/span.h"

namespace srsran {

/// CRC cyclic generator polynomials identifiers.
enum class crc_generator_poly : uint8_t {
  CRC24A = 0,
  CRC24B,
  CRC24C,
  CRC16,
  CRC11,
  CRC6,
  // etc...
};

/// Returns the CRC size from its identifier.
inline constexpr unsigned get_crc_size(crc_generator_poly poly)
{
  switch (poly) {
    case crc_generator_poly::CRC24A:
    case crc_generator_poly::CRC24B:
    case crc_generator_poly::CRC24C:
      return 24;
    case crc_generator_poly::CRC16:
      return 16;
    case crc_generator_poly::CRC11:
      return 11;
    case crc_generator_poly::CRC6:
    default:
      return 6;
  }
}

/// Checksum type.
using crc_calculator_checksum_t = unsigned;

/// \brief Calculates the CRC, as per TS38.212 Section 5.1. Interface.
class crc_calculator
{
public:
  virtual ~crc_calculator() = default;

  ///\brief Calculates the checksum from a byte buffer (8 packed bits in every input byte).
  ///\param[in] data Provides the byte buffer.
  ///\return The resulting CRC checksum.
  virtual crc_calculator_checksum_t calculate_byte(span<const uint8_t> data) const = 0;

  ///\brief Calculates the checksum from a bit buffer (1 bit in for every input byte).
  ///\param[in] data Provides the byte buffer.
  ///\return The resulting CRC checksum.
  virtual crc_calculator_checksum_t calculate_bit(span<const uint8_t> data) const = 0;

  ///\brief Calculates the checksum from a bit buffer.
  ///\param[in] data Input data.
  ///\return The resulting CRC checksum.
  virtual crc_calculator_checksum_t calculate(const bit_buffer& data) const = 0;

  ///\brief Returns the identifier of the generator polynomial.
  virtual crc_generator_poly get_generator_poly() const = 0;
};

} // namespace srsran
