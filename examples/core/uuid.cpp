#include <wite/core/uuid.hpp>
#include <wite/core/io.hpp>

#include <iostream>
#include <tuple>
#include <cassert>

int main() {
  // Create a new, random UUID
  const auto id_1 = wite::make_uuid();

  // UUIDs can be used with IO streams
  std::cout << "Created UUID with value " << id_1 << std::endl;

  // Create a new UUID from a string (defaults to 'D' format)
  const auto id_2_D = wite::uuid{"01234567-89AB-CDEF-0123-456789ABCDEF"};

  // Other formats can be used
  const auto id_2_B = wite::uuid{"{01234567-89AB-CDEF-0123-456789ABCDEF}", 'B'};
  const auto id_2_P = wite::uuid{"(01234567-89AB-CDEF-0123-456789ABCDEF)", 'P'};
  const auto id_2_N = wite::uuid{"0123456789ABCDEF0123456789ABCDEF", 'N'};
  const auto id_2_X = wite::uuid{"{0x01234567,0x89AB,0xCDEF,{0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF}}", 'X'};

  // These are all the same UUID value
  std::cout << "id_2_D == id_2_B: " << std::boolalpha << (id_2_D == id_2_B) << std::endl;
  std::cout << "id_2_D == id_2_P: " << std::boolalpha << (id_2_D == id_2_P) << std::endl;
  std::cout << "id_2_D == id_2_N: " << std::boolalpha << (id_2_D == id_2_N) << std::endl;
  std::cout << "id_2_D == id_2_X: " << std::boolalpha << (id_2_D == id_2_X) << std::endl;

  // A UUID can be converted to a string:
  const auto id_str_1 = id_1.str();
  std::cout << id_1 << " as a string = " << id_str_1 << std::endl;

  // ... and you can specify the format for the string
  const auto id_str_2 = id_2_D.str('X');
  std::cout << id_2_D << " as an X-format string = " << id_str_2 << std::endl;

  // If you have a buffer for a character array, then you can put the UUID into it
  auto uuid_char_array = std::array<char, 33>{};
  if (not id_1.into_c_str(uuid_char_array.data(), 33, 'N')) {
    std::cerr << "Failed to write id_1 to char buffer" << std::endl;
  }
  else {
    std::cout << "id_1 in a char buffer as an N-format string: " << uuid_char_array.data() << std::endl;
  }

  // These things are also available as free functions.  Here is how to use the free function to convert a UUID to a wide string
  const auto id_1_wide = wite::to_wstring(id_1, 'P');
  std::wcout << "id_1 (wide): " << id_1_wide << std::endl;

  // If you have a random engine that you want to use to generate random UUIDs, then you can provide it to the constructor
  auto rng = std::mt19937{std::random_device{}()};
  const auto id_3 = wite::uuid{rng};

  std::cout << "UUID generated with a specified random number generator: " << id_3 << std::endl;

  // If you're not using exceptions, then you can "try" to make a UUID
  const auto id_4 = wite::try_make_uuid("0123456789abcdef0123456789abcdef", 'n');
  assert(id_4.ok());

  std::cout << "UUID from try_make_uuid: " << id_4.value() << std::endl;

  // If the try fails, then an error code is returned instead
  const auto id_5 = wite::try_make_uuid("0123456789aXcdef0123456789abcdef", 'n');
  assert(id_5.is_error());

  std::cout << "try create UUID failed with code " << static_cast<int>(id_5.error()) << std::endl;

  return 0;
}
