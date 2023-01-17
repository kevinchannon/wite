/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#include <wite/string/fragment_string.hpp>

#include <iostream>
#include <string>

///////////////////////////////////////////////////////////////////////////////

const char* g_str = " and now a global part too!";

///////////////////////////////////////////////////////////////////////////////

const char* GetString() {
  return "FOO!";
}

///////////////////////////////////////////////////////////////////////////////

const char* DangerString(std::string&& str) {
  auto local_str = std::string(str);
  return local_str.c_str();
}

///////////////////////////////////////////////////////////////////////////////

int main() {
  // A motivating example
  // You often see this done with std::strings, which is doing lots of heap allocs.
  // The code below does zero heap allocs (except for in the cout itself).
  std::cout << "================================\n\n";

  constexpr auto base_filepath = wite::fragment_string{"/var/log/"};

  const auto component_1_logs = base_filepath + "component1/";
  const auto component_2_logs = base_filepath + "component2/";

  constexpr auto warning_logs_dir = "warning/";
  constexpr auto info_logs_dir    = "info/";

  constexpr auto log_filename = "logs.txt";

  const auto comp_1_warning_log_dir = component_1_logs + warning_logs_dir + log_filename;
  const auto comp_1_info_log_dir    = component_1_logs + info_logs_dir + log_filename;
  const auto comp_2_warning_log_dir = component_2_logs + warning_logs_dir + log_filename;
  const auto comp_2_info_log_dir    = component_2_logs + info_logs_dir + log_filename;

  std::cout << "Comp 1 warnings in: " << comp_1_warning_log_dir << std::endl;
  std::cout << "Comp 1 info in:     " << comp_1_info_log_dir << std::endl;
  std::cout << "Comp 2 warnings in: " << comp_2_warning_log_dir << std::endl;
  std::cout << "Comp 1 info in:     " << comp_2_info_log_dir << std::endl;

  std::cout << "\n================================\n\n";

  //
  // Other rnadom examples.
  //

  using namespace wite::string_literals;

  auto str = "This is a test string"_fs;
  std::cout << str << std::endl;

  auto str2 = str + " with an added part";
  std::cout << str2 << std::endl;

  auto str3 = str2 + g_str;
  std::cout << str3 << std::endl;

  auto str4 = wite::fragment_string{"The string function returns: "} + GetString();
  std::cout << str4 << std::endl;

  const auto s = str4.to_str();
  std::cout << "As a std::string? " << s << std::endl;

  //
  // You need to be careful, because the sort of thing illustrated below is totally possible. `s1` is defined here and then
  // moved into the `DangerString` function. Inside there, a local copy of the contents of `s1` is made. `DangerString` then
  // returns `.c_str()` on this local copy. When `DangerString` returns, the copy goes out of scope and the memory pointed
  // to by the return value is cleaned up. So, the memory is in an undefined state at this point.
  //
  // You can make `DangerString` safe by making the `local_str` variable inside it static.
  //
  auto str5      = wite::fragment_string{"What about this danger? "};
  auto s1        = std::string("BOO!");
  auto dangerStr = str5 + DangerString(std::move(s1));

  std::cout << dangerStr << std::endl;

  auto wstr = L"... and now a wiiiide string"_wfs;
  std::wcout << wstr << std::endl;

  auto wstr2 = wstr + L" with some more text";
  std::wcout << wstr2 << std::endl;

  return 0;
}

///////////////////////////////////////////////////////////////////////////////