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

#include <string>

namespace srsran {

class radio_uhd_device_type
{
public:
  enum class types {
    /// The device is not registered.
    UNKNOWN = 0,
    /// B2xx series models: B200mini, B200mini-i, B205mini-i, B210 and B200.
    B2xx,
    /// USRP Embedded series: E320, E312, E310 and E313.
    E3xx,
    /// N3x0 series models: N300 and N310.
    N3x0,
    /// N32x series models: N320 and N321.
    N32x,
    /// X3x0 series models: X300 and X310.
    X3x0,
    /// X410 model.
    X410
  };

  radio_uhd_device_type() = default;
  radio_uhd_device_type(types type_) : type(type_) {}
  radio_uhd_device_type(std::string mboard_name)
  {
    for (char& c : mboard_name) {
      c = std::toupper(c);
    }

    if (mboard_name.find("B2") != std::string::npos) {
      type = types::B2xx;
    } else if (mboard_name.find("X3") != std::string::npos) {
      type = types::X3x0;
    } else if (mboard_name.find("X4") != std::string::npos) {
      type = types::X410;
    } else if (mboard_name.find("N32") != std::string::npos) {
      type = types::N32x;
    } else if (mboard_name.find("N3") != std::string::npos) {
      type = types::N3x0;
    } else {
      type = types::UNKNOWN;
    }
  }
  std::string to_string()
  {
    switch (type) {
      case types::B2xx:
        return "uhd_b200";
      case types::E3xx:
        return "uhd_e3x0";
      case types::N3x0:
        return "uhd_n300";
      case types::N32x:
        return "uhd_n320";
      case types::X3x0:
        return "uhd_x300";
      case types::X410:
        return "uhd_x400";
      case types::UNKNOWN:
      default:
        return "uhd_unknown";
    }
  }
  bool operator==(const radio_uhd_device_type& other) const { return type == other.type; }
  bool operator==(const types& other) const { return type == other; }
       operator const types&() { return type; }

private:
  types type = types::UNKNOWN;
};
} // namespace srsran
