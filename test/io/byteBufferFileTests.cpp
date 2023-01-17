/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

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
#include <iterator>

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

  SECTION("read") {
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
  }

  SECTION("try_read") {
    SECTION("returns an error if the file cannot be opened") {
      auto bytes             = io::dynamic_byte_buffer(100);
      const auto read_result = io::try_read("not_a_file", bytes);
      REQUIRE(read_result.is_error());
      REQUIRE(io::read_error::file_not_found == read_result.error());
    }

    SECTION("returns the number of bytes read on success") {
      auto bytes        = io::dynamic_byte_buffer(100);
      const auto result = io::try_read(test_file.path, bytes);
      REQUIRE(result.ok());
      REQUIRE(std::string_view{TestFileMaker::default_content}.length() == result.value());

      SECTION("and the bytes read into the buffer are correct") {
        REQUIRE(TestFileMaker::default_content == to_string(bytes));
      }
    }
  }

  SECTION("write") {
    const auto bytes = io::dynamic_byte_buffer{io::byte{0x01},
                                               io::byte{0x02},
                                               io::byte{0x03},
                                               io::byte{0x04},
                                               io::byte{0x05},
                                               io::byte{0x06},
                                               io::byte{0x07},
                                               io::byte{0x08}};

    const auto get_file_bytes = [](const auto& path) {
      auto out = io::dynamic_byte_buffer{};

      std::ifstream byte_file{path, std::ios::binary};
      if (not byte_file.is_open()) {
        return out;
      }

      std::transform(std::istream_iterator<uint8_t>{byte_file}, {}, std::back_inserter(out), [](const auto& c) {
        return io::byte{c};
      });

      return out;
    };

    SECTION("writes specified number of bytes from a buffer to file") {

      io::write(test_file.path, 6, bytes);

      SECTION("writes the correct number of bytes to the file") {
        REQUIRE(6 == std::filesystem::file_size(test_file.path));

        SECTION("and the bytes have the expected values") {
          auto bytes_from_file = get_file_bytes(test_file.path);
          REQUIRE(6 == bytes_from_file.size());
          REQUIRE(std::equal(bytes.begin(), std::next(bytes.begin(), 6), bytes_from_file.begin()));
        }
      }
    }

    SECTION("writes whole buffer to file if number of bytes is not specified") {
      io::write(test_file.path, bytes);

      auto bytes_from_file = get_file_bytes(test_file.path);
      REQUIRE(bytes.size() == bytes_from_file.size());
      REQUIRE(bytes == bytes_from_file);
    }
  }
}