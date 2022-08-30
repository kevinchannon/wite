#include <wite/core/result.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

enum class ETestError {
  error_1,
  error_2,
  error_3
};

using TestResult_t = wite::result<uint32_t, ETestError>;

TEST_CASE("Result ok returns the expected value") {
  REQUIRE(TestResult_t{10}.ok());
  REQUIRE_FALSE(TestResult_t{ETestError::error_1}.ok());
}
