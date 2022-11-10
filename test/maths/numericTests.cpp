#include <wite/binascii/hexlify.hpp>
#include <wite/io/byte_buffer.hpp>
#include <wite/maths/numeric.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <string>
#include <ranges>

namespace {
struct non_pod_like {
  int* p{nullptr};

  non_pod_like() = delete;

  constexpr explicit non_pod_like(int i) : p{new int{i}} {}
  ~non_pod_like() { delete p; }

  constexpr non_pod_like(const non_pod_like& other) : p{new int{*other.p}} {}
  non_pod_like& operator=(const non_pod_like& other) {
    if (this != &other) {
      delete p;
      p = new int{*other.p};
    }

    return *this;
  }

  constexpr non_pod_like(non_pod_like&& other) noexcept : p{std::exchange(other.p, nullptr)} {}

  non_pod_like& operator=(non_pod_like&& other) noexcept {
    if (this != &other) {
      p = std::exchange(other.p, nullptr);
    }

    return *this;
  }

  constexpr bool operator<(const non_pod_like& other) const noexcept { return *p < *other.p; }
  constexpr bool operator<=(const non_pod_like& other) const noexcept { return *p <= *other.p; }
  constexpr bool operator>(const non_pod_like& other) const noexcept { return *p > *other.p; }
  constexpr bool operator>=(const non_pod_like& other) const noexcept { return *p >= *other.p; }
  constexpr bool operator==(const non_pod_like& other) const noexcept { return *p == *other.p; }
  constexpr bool operator!=(const non_pod_like& other) const noexcept { return *p != *other.p; }
};
}

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

TEST_CASE("max tests", "[maths]") {
  SECTION("integer values") {
    REQUIRE(100 == wite::maths::max(10, 20, 11, 30, 100));
  }

  SECTION("floating-point values") {
    REQUIRE(100.0 == wite::maths::max(10.0, 100.0, 0.0, -123.45, -100.0));
  }
}

TEST_CASE("minmax tests", "[maths]") {
  SECTION("non-variadic call", "[maths]") {
    SECTION("integer values") {
      REQUIRE(std::pair{0, 1} == wite::maths::minmax(0, 1));
      REQUIRE(std::pair{0, 1} == wite::maths::minmax(1, 0));
    }

    SECTION("floating-point values") {
      REQUIRE(std::pair{100.0, 123.45} == wite::maths::minmax(100.0, 123.45));
      REQUIRE(std::pair{100.0, 123.45} == wite::maths::minmax(123.45, 100.0));
    }

    SECTION("non-POD-like values are not copied") {
      const auto npl_1 = non_pod_like(10);
      const auto npl_2 = non_pod_like(11);

      SECTION("left is less") {
        const auto [min, max] = wite::maths::minmax(npl_1, npl_2);

        REQUIRE(npl_1.p == min.p);
        REQUIRE(npl_2.p == max.p);
      }

      SECTION("right is less") {
        const auto [min, max] = wite::maths::minmax(npl_2, npl_1);

        REQUIRE(npl_1.p == min.p);
        REQUIRE(npl_2.p == max.p);
      }
    }
  }

  SECTION("variadic call", "[maths]") {
    SECTION("integer values") {
      REQUIRE(std::pair{-2, 10} == wite::maths::minmax(0, 10, 5, -2, 1));
    }

    SECTION("floating-point values") {
      REQUIRE(std::pair{-23.8, 123.45} == wite::maths::minmax(0.0, -23.8, 100.0, 123.45, 52.5));
    }

    SECTION("non-POD-like values are not copied") {
      const auto npl_1 = non_pod_like(102);
      const auto npl_2 = non_pod_like(11);
      const auto npl_3 = non_pod_like(-5);
      const auto npl_4 = non_pod_like(25);

      const auto [min, max] = wite::maths::minmax(npl_1, npl_2, npl_3, npl_4);

      REQUIRE(npl_3.p == min.p);
      REQUIRE(npl_1.p == max.p);
    }
  }
}

TEST_CASE("interpolate tests", "[maths]") {
  SECTION("zero returns min") {
    REQUIRE(-1.0 == wite::maths::interpolate(0.0, -1.0, 1.0));
  }

  SECTION("one returns max") {
    REQUIRE(1.0 == wite::maths::interpolate(1.0, -1.0, 1.0));
  }

  SECTION("zero width range returns min") {
    REQUIRE(1.0 == wite::maths::interpolate(0.0, 1.0, 1.0));
  }

  SECTION("greater than one extrapolates range") {
    REQUIRE(4.0 == wite::maths::interpolate(2.0, 0.0, 2.0));
  }

  SECTION("less than one extrapolates range") {
    REQUIRE(-2.0 == wite::maths::interpolate(-1.0, 0.0, 2.0));
  }
}