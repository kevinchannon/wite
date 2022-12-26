#pragma once

#include "wite/common/constructor_macros.hpp"

#include <array>
#include <compare>

///////////////////////////////////////////////////////////////////////////////

#ifndef WITE_DEFAULT_UUID_FMT
#define WITE_DEFAULT_UUID_FMT 'D'
#endif

///////////////////////////////////////////////////////////////////////////////

namespace wite {

///////////////////////////////////////////////////////////////////////////////

constexpr auto default_uuid_format = WITE_DEFAULT_UUID_FMT;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
concept wite_uuid_like = requires(T& t) { t.data; };

template <typename T>
concept guid_like = requires(T& t) {
                      t.Data1;
                      t.Data2;
                      t.Data3;
                      t.Data4[0];
                      t.Data4[1];
                      t.Data4[2];
                      t.Data4[3];
                      t.Data4[4];
                      t.Data4[5];
                      t.Data4[6];
                      t.Data4[7];
                    };
template <typename T>
concept uuid_like = ((wite_uuid_like<T> or guid_like<T>) and sizeof(T) == 16);

///////////////////////////////////////////////////////////////////////////////

struct basic_uuid {
  using Storage_t = std::array<uint8_t, 16>;

  WITE_DEFAULT_CONSTRUCTORS(basic_uuid);

  explicit basic_uuid(Storage_t data) : data{data} {}

  constexpr auto operator<=>(const basic_uuid&) const noexcept = default;

  Storage_t data{};
};

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite

///////////////////////////////////////////////////////////////////////////////
