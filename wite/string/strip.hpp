#pragma once

#include <wite/string/trim.hpp>

#include <string>
#include <string_view>
#include <type_traits>

namespace wite::string {
  
template<typename Char_T>
std::basic_string<Char_T> strip(std::basic_string_view<Char_T> str)
{
  return trim_left(trim_right(str));
}

///////////////////////////////////////////////////////////////////////////////

//template<typename Char_T>
//std::basic_string<Char_T> strip(const Char_T* str)
//{
//  return trim_left(trim_right(std::basic_string_view<Char_T>{str}));
//}

///////////////////////////////////////////////////////////////////////////////
  
} // namespace wite::string

///////////////////////////////////////////////////////////////////////////////
