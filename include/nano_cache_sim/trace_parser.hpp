#ifndef TRACE_PARSER_HPP
#define TRACE_PARSER_HPP

#include <fmt/core.h>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace nano_cache_sim {

struct TraceOp {
  char type;  // 'L' or 'S'
  uint64_t addr;
};

inline std::vector<TraceOp> ParseTraceFile(const std::string& filename) {
  std::vector<TraceOp> ops;
  std::ifstream file(filename);

  if (!file.is_open()) {
    fmt::print(stderr, "Error: Could not open trace file {}\n", filename);
    return ops;
  }

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#') continue;

    std::stringstream ss(line);
    char type;
    std::string addr_str;

    // Expected format: L 0x1234 or S 0x1234
    if (ss >> type >> addr_str) {
      uint64_t addr = 0;
      try {
        addr = std::stoull(addr_str, nullptr, 16);
      } catch (...) {
        fmt::print(stderr, "Warning: Skipping invalid line: {}\n", line);
        continue;
      }
      ops.push_back({type, addr});
    }
  }

  return ops;
}

}  // namespace nano_cache_sim

#endif  // TRACE_PARSER_HPP
