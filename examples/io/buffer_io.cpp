#include <wite/io/byte_buffer.hpp>

#include <iostream>
#include <string_view>
#include <cassert>

///////////////////////////////////////////////////////////////////////////////

using namespace wite;

auto global_buffer = io::static_byte_buffer<100>{};

///////////////////////////////////////////////////////////////////////////////

namespace {
void title(std::string_view text) {
  std::cout << "===============================" << std::endl;
  std::cout << text << std::endl;
  std::cout << "===============================" << std::endl;
}
}  // namespace

///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// This is the simplest demonstration of reading from and writing to a buffer.
/// </summary>
void direct_read_write_of_buffer() {
  title("Direct buffer write/read");

  // Write then read an int.
  io::write(global_buffer, int{42});
  const auto a = io::read<int>(global_buffer);
  
  std::cout << "Read int " << a << " from buffer" << std::endl;

  // Write then read a double.
  io::write(global_buffer, 3.141562);
  const auto b = io::read<double>(global_buffer);

  std::cout << "Read double " << b << " from buffer" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// This is an example that writes two values to the buffer, managing the offsets of the values manually.
/// </summary>
void direct_read_write_with_offsets() {
  title("Direct buffer write/read with offsets");

  // Write two values (an int and a double) to the buffer.  Here we have to manage the offsets directly.
  // If you don't want to be bothered with this, then the easiest thing is to use a byte-buffer view
  // (see simple_buffer_write_read_operations)
  io::write(global_buffer, int{42});
  io::write({std::next(global_buffer.begin(), sizeof(int)), global_buffer.end()}, 3.141562);

  // When we read, then we again have to manage the offset of the elements ourselves. Again, using a
  // byte_read_buffer_view simplifies this.
  const auto a = io::read<int>(global_buffer);
  const auto b = io::read<double>({std::next(global_buffer.begin(), sizeof(int)), global_buffer.end()});

  std::cout << "Read int " << a << " from the buffer" << std::endl;
  std::cout << "Read double " << b << " from the buffer" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// An example that shows how to use an endianness encoding adapter to read a big-endian value from the buffer.
/// </summary>
void direct_read_write_with_endianness() {
  title("Direct buffer write/read with endianness");

  // Use the endianness encoding adapters to control the endianness:
  io::write(global_buffer, io::big_endian{uint64_t{0xFEDCBA9876543210}});

  // The same applies to reading, but it's done in the template parameter:
  const auto u64 = io::read<io::big_endian<uint64_t>>(global_buffer);

  std::cout << "Read 0x" << std::hex << u64 << " as a big-endian value" << std::endl;
}

/// <summary>
/// If you know you need to write/read several things in sequence from a buffer, that can be done in a single, variadic call.
/// </summary>
void direct_read_write_multiple_values() {
  title("Direct buffer write/read multiple values in a single call");

  // Some values to write.
  const auto a = uint32_t{0x12345678};
  const auto b = true;
  const auto c = double{1.618};

  // Write all the values to the buffer in one call (this function is variadic, so the number and types of items is arbitrary):
  io::write(global_buffer, a, b, c);

  // Read all the values back out in one call:
  const auto [d, e, f] = io::read<uint32_t, bool, double>(global_buffer);

  std::cout << "Read " << d << ", " << std::boolalpha << e << ", " << f << " from buffer" << std::endl;
}

/// <summary>
/// A mixture of endiannesses can be used to write/read multiple values.
/// </summary>
void direct_read_write_multiple_values_with_endianness() {
  title("Direct buffer write/read multiple values in a single call with mixed endiannesses");

  // Some values to write.
  const auto a = uint32_t{0x12345678};
  const auto b = uint64_t{0x234B509A83C061E7};
  const auto c = double{1.618};

  // Write all the values to the buffer in one call, but the middle value has a big-endian encoding:
  io::write(global_buffer, a, io::big_endian{b}, c);

  // Read all the values back out in one call (with the big-endian second, uint64_t value):
  const auto [d, e, f] = io::read<uint32_t, io::big_endian<uint64_t>, double>(global_buffer);

  std::cout << "Read 0x" << d << ", 0x" << e << ", " << f << " from buffer" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// This examples shows how exceptions can be used to detect write/read errors.
/// </summary>
void direct_read_write_handle_errors_with_exceptions() {
  title("Writing or reading past the end of a buffer will throw std::out_of_range");

  auto local_buffer = io::static_byte_buffer<3>{};

  try {
    io::write(local_buffer, uint32_t{42});
  }
  catch (const std::out_of_range& e) {
    std::cout << "Failed to write to buffer: " << e.what() << std::endl;
  }

  try {
    io::read<uint32_t>(local_buffer);
  } catch (const std::out_of_range& e) {
    std::cout << "Failed to read fromo buffer: " << e.what() << std::endl;
  }
}

///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// You might not be able to use exceptions in some circumstances. In that case, use the "try_" versions of functions.
/// </summary>
void direct_read_write_without_exceptions() {
  title("Direct write/read without exceptions");

  // Be careful, not hanndling the errors will likely abort your program!

  auto local_buffer = io::static_byte_buffer<7>{};

  const auto write_result_1 = io::try_write(local_buffer, uint32_t{42});
  if (write_result_1.is_error()) {
    // Do error things;
    return;
  }

  const auto a = io::try_read<uint32_t>(local_buffer);
  if (a.is_error()) {
    // Do error things.
    return;
  }

  std::cout << "Read " << a.value() << " from buffer" << std::endl;

  // Now an error case.
  const auto write_result_2 = io::try_write(local_buffer, uint64_t{42});
  if (write_result_2.is_error()) {
    std::cout << "Write failed with error code: " << static_cast<uint32_t>(write_result_2.error()) << std::endl;
  }

  const auto b = io::try_read<uint64_t>(local_buffer);
  if (b.is_error()) {
    std::cout << "Write failed with error code: " << static_cast<uint32_t>(write_result_2.error()) << std::endl;
  }
}

///////////////////////////////////////////////////////////////////////////////

void simple_buffer_write_read_operations() {
  title("Simple buffer read operations");

  // Write some things to the buffer.
  auto write_buf = io::byte_write_buffer_view{global_buffer};

  write_buf.write(uint16_t{10});
  write_buf.write(uint32_t{100});
  write_buf.write(uint64_t{100});
  write_buf.write(true);
  write_buf.write('x');
  write_buf.write(3.1415);
  write_buf.write(2.718f);

  // Write something with a given endianness
  write_buf.write(io::big_endian{0xDEADBEEF});

  std::cout << "Wrote " << write_buf.write_position()
            << " bytes to the buffer" << std::endl;

  std::cout << "===============================" << std::endl;

  // Read things out of the buffer
  auto read_buf = io::byte_read_buffer_view{global_buffer};

  std::cout << "Reading from buffer:" << std::endl;
  std::cout << "uint16 value = " << read_buf.read<uint16_t>() << std::endl;
  std::cout << "uint32 value = " << read_buf.read<uint32_t>() << std::endl;
  std::cout << "uint32 value = " << read_buf.read<uint64_t>() << std::endl;
  std::cout << "bool value = " << std::boolalpha << read_buf.read<bool>() << std::endl;
  std::cout << "char value = "   << read_buf.read<char>() << std::endl;
  std::cout << "double value = " << read_buf.read<double>() << std::endl;
  std::cout << "float value = "  << read_buf.read<float>() << std::endl;

  std::cout << "Big endian uint32 value = " << std::hex << "0x" << read_buf.read<io::big_endian<uint32_t>>() << std::endl;
}

///////////////////////////////////////////////////////////////////////////////

void direct_buffer_access_examples() {
  direct_read_write_of_buffer();
  std::cout << std::endl;

  direct_read_write_with_offsets();
  std::cout << std::endl;

  direct_read_write_with_endianness();
  std::cout << std::endl;

  direct_read_write_multiple_values();
  std::cout << std::endl;

  direct_read_write_multiple_values_with_endianness();
  std::cout << std::endl;

  direct_read_write_handle_errors_with_exceptions();
  std::cout << std::endl;

  direct_read_write_without_exceptions();
}

///////////////////////////////////////////////////////////////////////////////

void buffer_view_examples() {
  simple_buffer_write_read_operations();
}

///////////////////////////////////////////////////////////////////////////////

int main() {
  std::cout << "===============================" << std::endl;
  std::cout << "Buffer IO examples" << std::endl;

  direct_buffer_access_examples();
  std::cout << std::endl;

  buffer_view_examples();

  std::cout << "===============================" << std::endl;

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
