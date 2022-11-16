#include <wite/io/byte_buffer_file.hpp>

#include "../utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <filesystem>
#include <fstream>
#include <numeric>
#include <optional>
#include <string_view>
#include <algorithm>

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

  SECTION("Read whole file if no byte count is specified") {
    REQUIRE(TestFileMaker::default_content == to_string(io::read(test_file.path)));
  }

  SECTION("reading a non-existent path throws std::invalid_argument") {
    WITE_REQ_THROWS(io::read("not_a_file"), std::invalid_argument, "cannot read invalid path");
  }

  SECTION("reading too many bytes just reads until the end of the file") {
    REQUIRE(TestFileMaker::default_content == to_string(io::read(test_file.path, 18)));
  }

  SECTION("try_read returns an error if the file cannot be opened") {
    const auto read_result = io::try_read("not_a_file");
    REQUIRE(read_result.is_error());
    REQUIRE(io::read_error::file_not_found == read_result.error());
  }

  SECTION("Write specified number of bytes from a buffer to file") {
    const auto bytes = io::dynamic_byte_buffer{io::byte{0x01},
                                               io::byte{0x02},
                                               io::byte{0x03},
                                               io::byte{0x04},
                                               io::byte{0x05},
                                               io::byte{0x06},
                                               io::byte{0x07},
                                               io::byte{0x08}};

    io::write(test_file.path, 6, bytes);

    SECTION("writes the correct number of bytes to the file") {
      REQUIRE(6 == std::filesystem::file_size(test_file.path));

      SECTION("and the bytes have the expected values") {
        std::ifstream byte_file{test_file.path, std::ios::binary};
        REQUIRE(byte_file.is_open());
        auto bytes_from_file = io::dynamic_byte_buffer{};
        std::transform(std::istream_iterator<uint8_t>{byte_file}, {}, std::back_inserter(bytes_from_file), [](const auto& c) {
          return io::byte{c};
        });

        REQUIRE(6 == bytes_from_file.size());
        REQUIRE(std::equal(bytes.begin(), std::next(bytes.begin(), 6), bytes_from_file.begin()));
      }
    }
  }
}