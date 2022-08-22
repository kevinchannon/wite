#include <string>
#include <vector>
#include <numeric>
#include <algorithm>
#include <type_traits>

///////////////////////////////////////////////////////////////////////////////

namespace wite::string {

///////////////////////////////////////////////////////////////////////////////

namespace detail {
  template<typename Char_T>
  consteval auto space_character() {
    if constexpr (std::is_same_v<char, Char_T>) {
      return ' ';
    } else if constexpr (std::is_same_v<wchar_t, Char_T>) {
      return L' ';
    } else {
      static_assert(std::is_same_v<char, Char_T>, "Invalid character type");
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

template<typename Range_T>
typename std::basic_string<typename Range_T::value_type::value_type> join(
  const Range_T& strings,
  typename Range_T::value_type::value_type delimiter=detail::space_character<typename Range_T::value_type::value_type>())
{
  using Char_t = typename Range_T::value_type::value_type;
  
  switch (strings.size())
  {
  case 0:
    return std::basic_string<Char_t>{};
  case 1:
    return *strings.begin();
  default:;   // Fall through to main body of function.
  }

  const auto size = std::accumulate(strings.begin(), strings.end(), size_t(0), [](const auto& curr, const auto& str) {
    return curr + str.length();
    }) + strings.size() - 1;

  std::basic_string<Char_t> result(size, Char_t{});
  auto pos = std::copy(strings.begin()->begin(), strings.begin()->end(), result.begin());
  std::for_each(std::next(strings.begin()), strings.end(), [&pos, delimiter](const auto& s) {
    *pos++ = delimiter;
    pos = std::copy(s.begin(), s.end(), pos);
    });

  return result;
}


///////////////////////////////////////////////////////////////////////////////

}