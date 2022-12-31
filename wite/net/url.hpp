#pragma once

#include <wite/collections/stack_vector.hpp>
#include <wite/string/split.hpp>

#include <wite/env/environment.hpp>

#include <algorithm>
#include <array>
#include <string_view>
#include <sstream>

namespace wite::net {

class ip {
 public:
  constexpr explicit ip(std::string_view ip_string) : _data{_data_from_string(ip_string)} {}

  _WITE_NODISCARD std::string str() const {
    std::stringstream out;
    out << _data[0] << '.' << _data[1] << '.' << _data[2] << '.' << _data[3];
    return out.str();
  }

 private:
  using _storage_t = std::array<uint8_t, 4>;

  static constexpr _storage_t _data_from_string(std::string_view str) {
    const auto parts = string::split_to<wite::collections::stack_vector<std::string_view, 4>>(str.data(), '.');
    auto out         = _storage_t{};
    std::ranges::transform(parts, out.begin(), _from_string);
    return out;
  }

  _WITE_NODISCARD static uint8_t _from_string(std::string_view) { return 0; }

  _storage_t _data;
};

}  // namespace wite::net
