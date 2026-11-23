#include "converter.hpp"

#include <fstream>
#include <iostream>
#include <iterator>

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "usage: sinen_luau_to_cpp <input.luau> <output.hpp>\n";
    return 1;
  }

  std::ifstream input(argv[1], std::ios::binary);
  if (!input) {
    std::cerr << argv[1] << ": cannot open input\n";
    return 1;
  }
  const std::string source{std::istreambuf_iterator<char>(input),
                           std::istreambuf_iterator<char>()};
  input.close();

  const auto result = sinen::luau_to_cpp::convert(source, argv[1]);
  if (!result.error.empty()) {
    std::cerr << result.error << '\n';
    return 1;
  }

  std::ofstream output(argv[2], std::ios::binary);
  output << result.cpp;
  output.close();
  if (!output) {
    std::cerr << argv[2] << ": cannot write output\n";
    return 1;
  }
}
