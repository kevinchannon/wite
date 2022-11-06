#include <wite/binascii/hexlify.hpp>
#include <wite/io/byte_buffer.hpp>
#include <wite/maths/numeric.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <string>

TEST_CASE("next_value tests", "[maths]") {
  SECTION("next double") {
    const auto test_value = GENERATE(0.0, 1.0, 1.0e1, 1.0e2, 1.0e5, 1.0e10, 1.0e20, 1.0e50, 1.0e100);

    const auto increment_first_byte = [](double x) {
      uint8_t* first_byte = reinterpret_cast<uint8_t*>(&x);
      ++(*first_byte);

      return x;
    };

    SECTION(std::to_string(test_value)) {
      REQUIRE(increment_first_byte(test_value) == wite::maths::next_value(test_value));
    }
  }

  SECTION("next int") {
    const auto test_value = GENERATE(-10, -1, 0, 1, 10);

    SECTION(std::to_string(test_value)) {
      REQUIRE(test_value + 1 == wite::maths::next_value(test_value));
    }
  }
}

TEST_CASE("prev_value tests", "[maths]") {
  SECTION("prev double") {
    const auto test_value = GENERATE(1.0, 1.0e1, 1.0e2, 1.0e5, 1.0e10, 1.0e20, 1.0e50, 1.0e100);

    const auto increment_first_byte = [](double x) {
      uint8_t* first_byte = reinterpret_cast<uint8_t*>(&x);
      ++(*first_byte);

      return x;
    };

    SECTION(std::to_string(test_value)) {
      REQUIRE(test_value == wite::maths::prev_value(increment_first_byte(test_value)));
    }
  }

  SECTION("prev int") {
    const auto test_value = GENERATE(-10, -1, 0, 1, 10);

    SECTION(std::to_string(test_value)) {
      REQUIRE(test_value - 1 == wite::maths::prev_value(test_value));
    }
  }
}

TEST_CASE("min tests", "[maths]") {
  SECTION("integer values") {
    REQUIRE(10 == wite::maths::min(10, 20, 11, 30, 100));
  }

  SECTION("floating-point values") {
    REQUIRE(-123.45 == wite::maths::min(10.0, 100.0, 0.0, -123.45, -100.0));
  }
}