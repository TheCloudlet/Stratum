#include <memory>
#include <string>
#include <vector>

#include "nano_cache_sim/cache_sim.hpp"

using namespace nano_cache_sim;

int main() {
  // 1. Configuration
  using MemType = MainMemory<"MainMemory">;
  using L3Type = Cache<"L3", MemType, 8192, 16, 64, LRUPolicy, 20>;
  using L2Type = Cache<"L2", L3Type, 512, 8, 64, LRUPolicy, 10>;
  using L1Type = Cache<"L1", L2Type, 64, 8, 64, LRUPolicy, 4>;

  // Define Hierarchy Order for Stats Calculation
  std::vector<std::string> hierarchy = {"L1", "L2", "L3", "MainMemory"};

  auto cache_system = std::make_unique<L1Type>(100);

  // 2. Simulation Loop (Store Data)
  std::vector<AccessResult> history;
  std::vector<uint64_t> trace_addrs = {0x1000, 0x2000, 0x1000, 0x3000, 0x1000};

  for (auto addr : trace_addrs) {
    auto res = cache_system->Load(addr);
    history.push_back(res);
  }

  // 3. Print Stats
  PrintSimulationStats(history, hierarchy);
  PrintAccessLog(history, trace_addrs);

  return 0;
}
