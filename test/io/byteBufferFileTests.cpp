#include <wite/io/byte_buffer_file.hpp>

#include "../utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <filesystem>
#include <fstream>
#include <numeric>
#include <optional>
#include <string_view>

using namespace wite;

namespace {
struct TestFileMaker {
  constexpr static auto default_content = "Some test content";

  explicit TestFileMaker(std::filesystem::path path, std::optional<std::string_view> content = std::nullopt)
      : path{std::move(path)} {
    std::ofstream file{this->path};
    if (content) {
      file << *content;
    } else {
      file << default_content;
    }
  }

  ~TestFileMaker() {
    auto ec = std::error_code{};
    std::filesystem::remove(path, ec);
  }

  const std::filesystem::path path;
};

std::string to_string(const io::dynamic_byte_buffer& bytes) {
  return std::accumulate(bytes.begin(), bytes.end(), std::string{}, [](std::string&& current, io::byte next) {
    current.push_back(char(next));
    return current;
  });
}

}  // namespace

TEST_CASE("Byte buffer file tests", "[buffer_io]") {
  const auto test_file = TestFileMaker{"test_file_delete_me.bin"};

  SECTION("Read specified number of bytes from file") {
    REQUIRE(std::string(TestFileMaker::default_content).substr(0, 10) == to_string(io::read(test_file.path, 10)));
  }

  SECTION("Read whole file if no byte count is specified"){
    REQUIRE(TestFileMaker::default_content == to_string(io::read(test_file.path)));
  }

  SECTION("reading a non-existent path throws std::invalid_argument") {
    WITE_REQ_THROWS(io::read("not_a_file"), std::invalid_argument, "cannot read invalid path");
  }

  SECTION("reading too many bytes just reads until the end of the file") {
    REQUIRE(TestFileMaker::default_content == to_string(io::read(test_file.path, 18)));

    SECTION("...and returns the number of bytes read"){

    }
  }
}