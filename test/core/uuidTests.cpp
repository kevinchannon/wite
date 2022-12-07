#include <test/utils.hpp>
#include <wite/core/io.hpp>
#include <wite/core/uuid.hpp>
#include <wite/env/environment.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <chrono>
#include <iostream>
#include <sstream>

using namespace wite;
using namespace std::chrono_literals;

namespace {
struct FakeEngine {
  using result_type = uint64_t;
  constexpr static uint64_t min() { return 0; }
  constexpr static uint64_t max() { return 0xFFFFFFFFFFFFFFFF; }
  uint64_t operator()() { return 0x0123456789ABCDEF; }
};

struct GUID {
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t Data4[8];
};

}  // namespace

TEST_CASE("Uuid tests", "[core]") {
  SECTION("NULL UUID is all zero") {
    REQUIRE(uuid{0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}} == nulluuid);
  }

  SECTION("default constructed uuid is all zeros") {
    REQUIRE(uuid{0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}} == uuid{});
  }

  SECTION("construction") {
    SECTION("default construct is NULL") {}

    SECTION("create random UUID") {
      const auto id_1 = make_uuid();
      const auto id_2 = make_uuid();
      REQUIRE(id_1 != id_2);
    }

    SECTION("using a random number generator") {
      const auto id = uuid{FakeEngine{}};

      REQUIRE(uuid{0x89ABCDEF, 0x4567, 0x01A3, {0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01}} == id);
    }

    SECTION("Making a UUID doesn't take too long") {
      const auto start = std::chrono::high_resolution_clock::now();

      for (auto i = 0u; i < 100'000; ++i) {
        const auto id = make_uuid();
        (void)id;
      }

      const auto duration = std::chrono::high_resolution_clock::now() - start;

      REQUIRE(100ms > std::chrono::duration_cast<std::chrono::milliseconds>(duration));
    }

    SECTION("from string") {
      SECTION("throws std::invalid_argument if the string is too short") {
        WITE_REQ_THROWS(uuid{"01234567-89AB-CDEF-0123-456789ABCDE"}, std::invalid_argument, "Invalid UUID format");
      }

      SECTION("throws std::invalid_argument if the string is too long") {
        WITE_REQ_THROWS(uuid{"01234567-89AB-CDEF-0123-456789ABCDEF0"}, std::invalid_argument, "Invalid UUID format");
      }

      SECTION("throws std::invalid_argument if the string doesn't contain non-hex or dash characters") {
        WITE_REQ_THROWS(uuid{"0123456X-89AB-CDEF-0123-456789ABCDEF"}, std::invalid_argument, "Invalid UUID format");
        WITE_REQ_THROWS(uuid{"01234567-89AX-CDEF-0123-456789ABCDEF"}, std::invalid_argument, "Invalid UUID format");
        WITE_REQ_THROWS(uuid{"01234567-89AB-CDEF-0123_456789ABCDEF"}, std::invalid_argument, "Invalid UUID format");
        WITE_REQ_THROWS(uuid{"01234567-89AB-CDEX-0123-456789ABCDEF"}, std::invalid_argument, "Invalid UUID format");
        WITE_REQ_THROWS(uuid{"01234567-89AB-CDEX-0123-456789AXCDEF"}, std::invalid_argument, "Invalid UUID format");
      }
    }
  }

  const auto id = uuid{0x01234567, 0x89AB, 0xCDEF, {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF}};

  SECTION("Write uuid into a C-string") {
    SECTION("narrow chars") {
      auto [test_name, expected_output, size, format_char] = GENERATE(
          table<const char*, const char*, size_t, char>({{"D-format", "01234567-89AB-CDEF-0123-456789ABCDEF", 37, 'D'},
                                                         {"N-format", "0123456789ABCDEF0123456789ABCDEF", 33, 'N'},
                                                         {"B-format", "{01234567-89AB-CDEF-0123-456789ABCDEF}", 39, 'B'},
                                                         {"P-format", "(01234567-89AB-CDEF-0123-456789ABCDEF)", 39, 'P'},
                                                         {"X-format", "{0x01234567,0x89AB,0xCDEF,{0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF}}", 70, 'X'}}));

      char buffer[70] = {};

      SECTION(test_name) {
        SECTION("succeeds if the buffer is sufficiently sized") {
          REQUIRE(id.into_c_str(buffer, size, format_char));
          REQUIRE(expected_output == std::string{buffer});
        }

        SECTION("fails if the buffer is too small") {
          REQUIRE_FALSE(id.into_c_str(buffer, size - 1, format_char));
        }
      }
    }

    SECTION("wide chars") {
      auto [test_name, expected_output, size, format_char] = GENERATE(table<const char*, const wchar_t*, size_t, char>(
          {{"D-format", L"01234567-89AB-CDEF-0123-456789ABCDEF", 37, 'D'},
           {"N-format", L"0123456789ABCDEF0123456789ABCDEF", 33, 'N'},
           {"B-format", L"{01234567-89AB-CDEF-0123-456789ABCDEF}", 39, 'B'},
           {"P-format", L"(01234567-89AB-CDEF-0123-456789ABCDEF)", 39, 'P'},
           {"X-format", L"{0x01234567,0x89AB,0xCDEF,{0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF}}", 70, 'X'}}));

      wchar_t buffer[70] = {};

      SECTION(test_name) {
        SECTION("succeeds if the buffer is sufficiently sized") {
          REQUIRE(id.into_c_str(buffer, size, format_char));
          REQUIRE(expected_output == std::wstring{buffer});
        }

        SECTION("fails if the buffer is too small") {
          REQUIRE_FALSE(id.into_c_str(buffer, size - 1, format_char));
        }
      }
    }
  }

  SECTION("convert to std::string") {
    SECTION("narrow string") {
      REQUIRE("01234567-89AB-CDEF-0123-456789ABCDEF" == id.str());
    }

    SECTION("wide string") {
      REQUIRE(L"01234567-89AB-CDEF-0123-456789ABCDEF" == id.wstr());
    }
  }

  SECTION("convert to std::string via free function") {
    SECTION("narrow string") {
      REQUIRE("01234567-89AB-CDEF-0123-456789ABCDEF" == to_string(id));

      SECTION("works for some alternative UUID implementation") {
        const auto guid = GUID{0x01234567, 0x89AB, 0xCDEF, {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF}};
        REQUIRE("01234567-89AB-CDEF-0123-456789ABCDEF" == to_string(guid));
      }
    }

    SECTION("wide string") {
      REQUIRE(L"01234567-89AB-CDEF-0123-456789ABCDEF" == to_wstring(id));

      SECTION("works for some alternative UUID implementation") {
        const auto guid = GUID{0x01234567, 0x89AB, 0xCDEF, {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF}};
        REQUIRE(L"01234567-89AB-CDEF-0123-456789ABCDEF" == to_wstring(guid));
      }
    }
  }
}

TEST_CASE("Uuid IO tests", "[core]") {
  const auto id = uuid{0x01234567, 0x89AB, 0xCDEF, {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF}};

  std::stringstream ss;
  ss << id;

  REQUIRE("01234567-89AB-CDEF-0123-456789ABCDEF" == ss.str());
}
