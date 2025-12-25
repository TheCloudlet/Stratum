#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <fmt/core.h>

#include <memory>
#include <string>
#include <vector>

#include "nano_cache_sim/cache_sim.hpp"
#include "nano_cache_sim/trace_parser.hpp"

namespace nano_cache_sim {

/**
 * RunTraceSimulation
 *
 * Simulates the cache system with a given trace file.
 *
 * Template Args:
 *   CacheSystem: The Top-Level Cache Type (e.g. L1Type)
 *
 * Function Args:
 *   trace_name: Human readable name for output
 *   filepath: Path to the trace file
 *   hierarchy: List of cache level names for stats aggregation
 *   lat: Hit latency/access time for the top-level cache (default 100 if
 * CacheSystem constructor takes it? Wait, your Cache implementations vary.
 *        L1Type usually takes the MemLatency if constructed recursively?
 *        Actually L1 constructor usually takes args for the *next* layer or
 * just latency?
 *
 *        Let's assume CacheSystem is constructed with TOP LEVEL latency or
 * arguments. However, in your main.cpp you did: `auto cache_system =
 * std::make_unique<L1Type>(100);` The arg `100` was passed down to MainMemory
 * ultimately.
 *
 *        To support generic construction, we can allow passing constructor
 * args. But for now, sticking to your `(100)` convention or taking it as an arg
 * is fine.
 * )
 */
template <typename CacheSystem>
void RunTraceSimulation(const std::string& trace_name,
                        const std::string& filepath,
                        const std::vector<std::string>& hierarchy,
                        size_t mem_latency = 100) {
  fmt::print("\n=========================================================\n");
  fmt::print("Running Simulation: {} ({})\n", trace_name, filepath);
  fmt::print("=========================================================\n");

  auto ops = ParseTraceFile(filepath);
  if (ops.empty()) {
    fmt::print("No operations to simulate for {}\n", trace_name);
    return;
  }

  // Initialize Cache System
  // Assuming constructor takes size_t for latency of the *lowest* level
  // (Memory) or follows the pattern `Cache(Args...)`.
  auto cache_system = std::make_unique<CacheSystem>(mem_latency);

  std::vector<AccessResult> history;
  std::vector<uint64_t> trace_addrs;

  for (const auto& op : ops) {
    AccessResult res;
    if (op.type == 'L') {
      res = cache_system->Load(op.addr);
    } else {
      res = cache_system->Store(op.addr);
    }
    history.push_back(res);
    trace_addrs.push_back(op.addr);
  }

  PrintSimulationStats(history, hierarchy);

  if (history.size() <= 20) {
    PrintAccessLog(history, trace_addrs);
  } else {
    fmt::print("\n(Detailed history hidden for large trace: {} ops)\n",
               history.size());
  }
}

}  // namespace nano_cache_sim

#endif  // SIMULATION_HPP
