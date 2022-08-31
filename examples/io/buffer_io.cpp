#include <wite/io/byte_buffer.hpp>

#include <iostream>

using namespace wite;

auto global_buffer = io::static_byte_buffer<100>{};

void simple_buffer_write_read_operations() {
  // Write some things to the buffer.
  auto write_buf = io::byte_write_buffer_view{global_buffer};

  io::write(write_buf, 10);
  io::write(write_buf, true);
  io::write(write_buf, 'x');
  io::write(write_buf, 3.1415);
  io::write(write_buf, 2.718f);

  // Write something with a given endianness
  io::write(write_buf, io::big_endian{0xDEADBEEF});

  // Read things out of the buffer
  auto read_buf = io::byte_read_buffer_view{global_buffer};

  std::cout << "int value = "    << io::read<int>(read_buf) << std::endl;
  std::cout << "bool value = "   << std::boolalpha << io::read<bool>(read_buf) << std::endl;
  std::cout << "char value = "   << io::read<char>(read_buf) << std::endl;
  std::cout << "double value = " << io::read<double>(read_buf) << std::endl;
  std::cout << "float value = "  << io::read<float>(read_buf) << std::endl;

  std::cout << "Big endian uint32 value = " << std::hex << "0x" << io::read<io::big_endian<uint32_t>>(read_buf) << std::endl;
}

int main() {
  simple_buffer_write_read_operations();

  return 0;
}