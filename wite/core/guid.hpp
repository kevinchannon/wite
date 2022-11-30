#pragma once

#include <compare>

namespace wite {

/// <summary>
/// A definition of a globally unique identifier that is compatible with the one in Windows' guiddef.h header.
/// </summary>
struct GUID {
  unsigned long Data1{};
  unsigned short Data2{};
  unsigned short Data3{};
  unsigned char Data4[8]{};

  constexpr auto operator<=>(const GUID&) const noexcept = default;
};

}  // namespace wite
