#include <wite/maths/numeric.hpp>
#include <wite/io/byte_buffer.hpp>
#include <wite/binascii/hexlify.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <string>
#include <format>

TEST_CASE("next_value tests", "[maths]") {
  SECTION("next double") {
    const auto [testValue, expectedNextValue ]
      = GENERATE(table<double, double>({
      {0.0, std::numeric_limits<double>::epsilon()},
        {1.0, std::numeric_limits<double>::epsilon()},
        {1.0e1, std::numeric_limits<double>::epsilon()},
        {1.0e2, std::numeric_limits<double>::epsilon()},
        {1.0e5, std::numeric_limits<double>::epsilon()},
        {1.0e10, std::numeric_limits<double>::epsilon()},
        {1.0e20, std::numeric_limits<double>::epsilon()},
        {1.0e50, std::numeric_limits<double>::epsilon()},
        {1.0e100, std::numeric_limits<double>::epsilon()}
        }));

    SECTION(std::format("{}: {} -> {}" , testValue, wite::binascii::hexlify(wite::io::to_bytes(testValue)), wite::binascii::hexlify(wite::io::to_bytes(wite::maths::next_value(testValue))))) {
      REQUIRE(expectedNextValue == wite::maths::next_value(testValue));
    }
  }
}
