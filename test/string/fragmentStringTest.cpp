#include <wite/string/fragment_string.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace wite;

TEST_CASE("fragment_string tests", "[string]") {
  SECTION("construct") {
    SECTION("from a single C-string") {
      const auto fs = string::fragment_string{"hello!"};

      REQUIRE(6 == fs.length());
      REQUIRE(std::string{"hello!"} == fs.to_str());
    }
  }

  SECTION("concatenate") {
    const auto fs_1 = string::fragment_string{"me "};
    REQUIRE(std::string{"me "} == fs_1.to_str());

    const auto fs_2 = fs_1 + "thinks ";
    REQUIRE(std::string{"me thinks "} == fs_2.to_str());

    const auto fs_3 = string::fragment_string{"it "};
    REQUIRE(std::string{"it "} == fs_3.to_str());

    const auto fs_4 = fs_2 + fs_3;
    REQUIRE(std::string{"me thinks it "} == fs_4.to_str());

    const auto fs_5 = fs_4 + "is " + "a " + "weasle";
    REQUIRE(std::string{"me thinks it is a weasle"} == fs_5.to_str());
  }

  SECTION("length and size match") {
    const auto fs = string::fragment_string{"Lorem ipsum dolor sit"};
    REQUIRE(fs.length() == fs.size());
  }
}
