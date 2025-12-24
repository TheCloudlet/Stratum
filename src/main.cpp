#include <fmt/core.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "nano_cache_sim/cache_sim.hpp"

using namespace nano_cache_sim;

struct CacheStats {
  size_t hits = 0;
  size_t misses = 0;
  size_t total_latency = 0;
};

int main() {
  using MemType = MainMemory<"MainMemory">;
  using L3Type = Cache<"L3", MemType, 8192, 16, 64, LRUPolicy, 20>;
  using L2Type = Cache<"L2", L3Type, 512, 8, 64, LRUPolicy, 10>;
  using L1Type = Cache<"L1", L2Type, 64, 8, 64, LRUPolicy, 4>;

  // Define Hierarchy Order for Stats Calculation
  std::vector<std::string> hierarchy = {"L1", "L2", "L3", "MainMemory"};

  fmt::print("Constructing Cache Hierarchy...\n");
  auto cache_system = std::make_unique<L1Type>(100);
  fmt::print("Starting Simulation...\n");

  // 1. Simulation Loop (Log results)
  std::vector<AccessResult> history;
  std::vector<uint64_t> trace_addrs = {0x1000, 0x2000, 0x1000, 0x3000, 0x1000};

  for (auto addr : trace_addrs) {
    auto res = cache_system->Load(addr);
    history.push_back(res);
  }

  // 2. Post-Processing (Aggregation)
  std::map<std::string, CacheStats> stats_db;

  for (const auto& res : history) {
    bool hit_found = false;
    for (const auto& level_name : hierarchy) {
      if (level_name == res.hit_level) {
        stats_db[level_name].hits++;
        stats_db[level_name].total_latency += res.total_cycles;
        hit_found = true;
        break;  // Stop, we found the hit level
      } else {
        // If we haven't found the hit yet, it means we missed in this level
        stats_db[level_name].misses++;
      }
    }
    if (!hit_found) {
      fmt::print(stderr, "Error: Hit level {} not in hierarchy def!\n", res.hit_level);
    }
  }

  // Report Results
  fmt::print("\n=== Simulation Results (Aggregated) ===\n");
  fmt::print("{:<15} {:<10} {:<10} {:<20}\n", "Level", "Hits", "Misses",
             "Avg Latency (cyc)");

  for (const auto& level_name : hierarchy) {
    const auto& s = stats_db[level_name];
    double avg_lat = 0.0;
    if (s.hits > 0) avg_lat = (double)s.total_latency / s.hits;

    fmt::print("{:<15} {:<10} {:<10} {:<20.0f}\n", level_name, s.hits, s.misses,
               avg_lat);
  }

  fmt::print("\n=== Detailed History ===\n");
  for (size_t i = 0; i < history.size(); ++i) {
    fmt::print("Access[{}] Addr={:x} Hit={} Cyc={}\n", i, trace_addrs[i],
               history[i].hit_level, history[i].total_cycles);
  }

  return 0;
}
