#include <wite/io/byte_buffer.hpp>

#include <iostream>

using namespace wite;

auto global_buffer = io::static_byte_buffer<100>{};

void simple_buffer_write_read_operations() {
  std::cout << "===============================" << std::endl;
  std::cout << "Simple buffer read operations" << std::endl;
  std::cout << "===============================" << std::endl;

  // Write some things to the buffer.
  auto write_buf = io::byte_write_buffer_view{global_buffer};

  io::write(write_buf, uint16_t{10});
  io::write(write_buf, uint32_t{100});
  io::write(write_buf, uint64_t{100});
  io::write(write_buf, true);
  io::write(write_buf, 'x');
  io::write(write_buf, 3.1415);
  io::write(write_buf, 2.718f);

  // Write something with a given endianness
  io::write(write_buf, io::big_endian{0xDEADBEEF});

  std::cout << "Wrote " << std::distance(write_buf.data.begin(), write_buf.write_position)
            << " bytes to the buffer" << std::endl;

  std::cout << "===============================" << std::endl;

  // Read things out of the buffer
  auto read_buf = io::byte_read_buffer_view{global_buffer};

  std::cout << "Reading from buffer:" << std::endl;
  std::cout << "uint16 value = " << io::read<uint16_t>(read_buf) << std::endl;
  std::cout << "uint32 value = " << io::read<uint32_t>(read_buf) << std::endl;
  std::cout << "uint32 value = " << io::read<uint64_t>(read_buf) << std::endl;
  std::cout << "bool value = "   << std::boolalpha << io::read<bool>(read_buf) << std::endl;
  std::cout << "char value = "   << io::read<char>(read_buf) << std::endl;
  std::cout << "double value = " << io::read<double>(read_buf) << std::endl;
  std::cout << "float value = "  << io::read<float>(read_buf) << std::endl;

  std::cout << "Big endian uint32 value = " << std::hex << "0x" << io::read<io::big_endian<uint32_t>>(read_buf) << std::endl;
}

int main() {
  std::cout << "===============================" << std::endl;
  std::cout << "Buffer IO examples" << std::endl;

  simple_buffer_write_read_operations();

  std::cout << "===============================" << std::endl;

  return 0;
}