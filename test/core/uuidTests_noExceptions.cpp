#define WITE_NO_EXCEPTIONS

#include <test/utils.hpp>
#include <wite/core/io.hpp>
#include <wite/core/uuid.hpp>
#include <wite/env/environment.hpp>
#include "wite/core/uuid/uuid_functions.hpp"

#include <catch2/catch_test_macros.hpp>

#include <chrono>

using namespace wite;
using namespace std::chrono_literals;

namespace {

struct GUID {
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t Data4[8];
};

}  // namespace

TEST_CASE("Uuid tests (no except)", "[core]") {
  SECTION("NULL UUID is all zero") {
    REQUIRE(uuid{{}} == nulluuid);
  }

  SECTION("default constructed uuid is all zeros") {
    REQUIRE(uuid{{}} == uuid{});
  }

  SECTION("construction") {
    SECTION("default construct is NULL") {}

    SECTION("create random UUID") {
      const auto id_1 = make_uuid();
      const auto id_2 = make_uuid();
      REQUIRE(id_1 != id_2);
    }

    SECTION("using a random number generator") {
      const auto id = uuid{test::FakeRandomEngine{}};
      auto uuid_data =
          uuid::Storage_t{0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x43, 0x01, 0xAF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01};

      REQUIRE(uuid{uuid_data} == id);
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
      SECTION("D-format") {
        SECTION("succeeds for valid string") {
          const auto expected =
              uuid{{0x67, 0x45, 0x23, 0x01, 0xAB, 0x89, 0xEF, 0xCD, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF}};
          const auto actual = try_make_uuid("01234567-89AB-CDEF-0123-456789ABCDEF");
          REQUIRE(actual.ok());
          REQUIRE(expected == actual.value());
        }

        SECTION("returns invalid_uuid_format if the string is too short") {
          REQUIRE(try_make_uuid("01234567-89AB-CDEF-0123-456789ABCDE").is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("01234567-89AB-CDEF-0123-456789ABCDE").error());
        }

        SECTION("returns invalid_uuid_format if the string is too long") {
          REQUIRE(try_make_uuid("01234567-89AB-CDEF-0123-456789ABCDEF0").is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("01234567-89AB-CDEF-0123-456789ABCDEF0").error());
        }

        SECTION("returns invalid_uuid_format if the string contains non-hex or dash characters") {
          REQUIRE(try_make_uuid("0123456X-89AB-CDEF-0123-456789ABCDEF").is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("0123456X-89AB-CDEF-0123-456789ABCDEF").error());

          REQUIRE(try_make_uuid("01234567-89AX-CDEF-0123-456789ABCDEF").is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("01234567-89AX-CDEF-0123-456789ABCDEF").error());

          REQUIRE(try_make_uuid("01234567-89AB-CDEF-0123_456789ABCDEF").is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("01234567-89AB-CDEF-0123_456789ABCDEF").error());

          REQUIRE(try_make_uuid("01234567-89AB-CDEX-0123-456789ABCDEF").is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("01234567-89AB-CDEX-0123-456789ABCDEF").error());

          REQUIRE(try_make_uuid("01234567-89AB-CDEX-0123-456789AXCDEF").is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("01234567-89AB-CDEX-0123-456789AXCDEF").error());
        }
      }

      SECTION("N-format") {
        SECTION("succeeds for valid string") {
          const auto expected =
              uuid{{0x67, 0x45, 0x23, 0x01, 0xAB, 0x89, 0xEF, 0xCD, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF}};
          const auto actual = try_make_uuid("0123456789ABCDEF0123456789ABCDEF", 'N');
          REQUIRE(actual.ok());
          REQUIRE(expected == actual.value());
        }

        SECTION("returns invalid_uuid_format if the string is too short") {
          REQUIRE(try_make_uuid("0123456789ABCDEF0123456789ABCDE", 'N').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("0123456789ABCDEF0123456789ABCDE", 'N').error());
        }

        SECTION("returns invalid_uuid_format if the string is too long") {
          REQUIRE(try_make_uuid("0123456789ABCDEF0123456789ABCDEF0", 'N').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("0123456789ABCDEF0123456789ABCDEF0", 'N').error());
        }

        SECTION("returns invalid_uuid_format if the string contains non-hex characters") {
          REQUIRE(try_make_uuid("0123456X89ABCDEF0123456789ABCDEF", 'N').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("0123456X89ABCDEF0123456789ABCDEF", 'N').error());
        }
      }

      SECTION("B-format") {
        SECTION("succeeds for valid string") {
          const auto expected =
              uuid{{0x67, 0x45, 0x23, 0x01, 0xAB, 0x89, 0xEF, 0xCD, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF}};
          const auto actual = try_make_uuid("{01234567-89AB-CDEF-0123-456789ABCDEF}", 'B');
          REQUIRE(actual.ok());
          REQUIRE(expected == actual.value());
        }

        SECTION("returns invalid_uuid_format if the string is too short") {
          REQUIRE(try_make_uuid("{01234567-89AB-CDEF-0123-456789ABCDE}", 'B').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("{01234567-89AB-CDEF-0123-456789ABCDE}", 'B').error());
        }

        SECTION("returns invalid_uuid_format if the string is too long") {
          REQUIRE(try_make_uuid("{01234567-89AB-CDEF-0123-456789ABCDEF0}", 'B').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("{01234567-89AB-CDEF-0123-456789ABCDEF0}", 'B').error());
        }

        SECTION("returns invalid_uuid_format if the string contains non-hex or dash characters") {
          REQUIRE(try_make_uuid("{0123456X-89AB-CDEF-0123-456789ABCDEF}", 'B').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("{0123456X-89AB-CDEF-0123-456789ABCDEF}", 'B').error());

          REQUIRE(try_make_uuid("{01234567-89AX-CDEF-0123-456789ABCDEF}", 'B').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("{01234567-89AX-CDEF-0123-456789ABCDEF}", 'B').error());

          REQUIRE(try_make_uuid("{01234567-89AB-CDEF-0123_456789ABCDEF}", 'B').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("{01234567-89AB-CDEF-0123_456789ABCDEF}", 'B').error());

          REQUIRE(try_make_uuid("{01234567-89AB-CDEX-0123-456789ABCDEF}", 'B').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("{01234567-89AB-CDEX-0123-456789ABCDEF}", 'B').error());

          REQUIRE(try_make_uuid("{01234567-89AB-CDEX-0123-456789AXCDEF}", 'B').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("{01234567-89AB-CDEX-0123-456789AXCDEF}", 'B').error());
        }

        SECTION("returns invalid_uuid_format if string is empty") {
          REQUIRE(try_make_uuid("", 'B').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("", 'B').error());
        }

        SECTION("returns invalid_uuid_format if the first or last character is not a brace") {
          REQUIRE(try_make_uuid("?01234567-89AB-CDEF-0123-456789ABCDEF}", 'B').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("?01234567-89AB-CDEF-0123-456789ABCDEF}", 'B').error());

          REQUIRE(try_make_uuid("{01234567-89AB-CDEF-0123-456789ABCDEF?", 'B').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("{01234567-89AB-CDEF-0123-456789ABCDEF?", 'B').error());
        }
      }

      SECTION("P-format") {
        SECTION("succeeds for valid string") {
          const auto expected =
              uuid{{0x67, 0x45, 0x23, 0x01, 0xAB, 0x89, 0xEF, 0xCD, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF}};
          const auto actual = try_make_uuid("(01234567-89AB-CDEF-0123-456789ABCDEF)", 'P');
          REQUIRE(actual.ok());
          REQUIRE(expected == actual.value());
        }

        SECTION("returns invalid_uuid_format if the string is too short") {
          REQUIRE(try_make_uuid("(01234567-89AB-CDEF-0123-456789ABCDE)", 'P').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("(01234567-89AB-CDEF-0123-456789ABCDE)", 'P').error());
        }

        SECTION("returns invalid_uuid_format if the string is too long") {
          REQUIRE(try_make_uuid("(01234567-89AB-CDEF-0123-456789ABCDEF0)", 'P').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("(01234567-89AB-CDEF-0123-456789ABCDEF0)", 'P').error());
        }

        SECTION("returns invalid_uuid_format if the string contains non-hex or dash characters") {
          REQUIRE(try_make_uuid("(0123456X-89AB-CDEF-0123-456789ABCDEF)", 'P').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("(0123456X-89AB-CDEF-0123-456789ABCDEF)", 'P').error());

          REQUIRE(try_make_uuid("(01234567-89AX-CDEF-0123-456789ABCDEF)", 'P').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("(01234567-89AX-CDEF-0123-456789ABCDEF)", 'P').error());

          REQUIRE(try_make_uuid("(01234567-89AB-CDEF-0123_456789ABCDEF)", 'P').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("(01234567-89AB-CDEF-0123_456789ABCDEF)", 'P').error());

          REQUIRE(try_make_uuid("(01234567-89AB-CDEX-0123-456789ABCDEF)", 'P').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("(01234567-89AB-CDEX-0123-456789ABCDEF)", 'P').error());

          REQUIRE(try_make_uuid("(01234567-89AB-CDEX-0123-456789AXCDEF)", 'P').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("(01234567-89AB-CDEX-0123-456789AXCDEF)", 'P').error());
        }

        SECTION("returns invalid_uuid_format if string is empty") {
          REQUIRE(try_make_uuid("", 'P').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("", 'P').error());
        }

        SECTION("returns invalid_uuid_format if the first or last character is not a brace") {
          REQUIRE(try_make_uuid("?01234567-89AB-CDEF-0123-456789ABCDEF)", 'P').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("?01234567-89AB-CDEF-0123-456789ABCDEF)", 'P').error());

          REQUIRE(try_make_uuid("(01234567-89AB-CDEF-0123-456789ABCDEF?", 'P').is_error());
          REQUIRE(make_uuid_error::invalid_uuid_format == try_make_uuid("(01234567-89AB-CDEF-0123-456789ABCDEF?", 'P').error());
        }
      }

      SECTION("returns invalid_uuid_format_type if the format specifier is invalid") {
        REQUIRE(try_make_uuid("{01234567-89AB-CDEF-0123-456789ABCDEF}", '?').is_error());
        REQUIRE(make_uuid_error::invalid_uuid_format_type ==
                try_make_uuid("{01234567-89AB-CDEF-0123-456789ABCDEF}", '?').error());
      }
    }
  }

  auto uuid_data =
      uuid::Storage_t{0x67, 0x45, 0x23, 0x01, 0xab, 0x89, 0xef, 0xcd, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
  const auto id = uuid{uuid_data};

  SECTION("Write uuid into a C-string") {
    SECTION("narrow chars") {
      char buffer[39] = {};

      SECTION("succeeds if the buffer is sufficiently sized") {
        REQUIRE(id.into_c_str(buffer, 37));

        REQUIRE("01234567-89AB-CDEF-0123-456789ABCDEF" == std::string{buffer});
      }

      SECTION("fails if the buffer is too small") {
        REQUIRE_FALSE(id.into_c_str(buffer, 36));
      }
    }

    SECTION("wide chars") {
      wchar_t buffer[39] = {};

      SECTION("succeeds if the buffer is sufficiently sized") {
        REQUIRE(id.into_c_str(buffer, 37));

        REQUIRE(L"01234567-89AB-CDEF-0123-456789ABCDEF" == std::wstring{buffer});
      }

      SECTION("fails if the buffer is too small") {
        REQUIRE_FALSE(id.into_c_str(buffer, 36));
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

TEST_CASE("Uuid IO tests  (no except)", "[core]") {
  auto uuid_data =
      uuid::Storage_t{0x67, 0x45, 0x23, 0x01, 0xab, 0x89, 0xef, 0xcd, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
  const auto id = uuid{uuid_data};

  std::stringstream ss;
  ss << id;

  REQUIRE("01234567-89AB-CDEF-0123-456789ABCDEF" == ss.str());
}
