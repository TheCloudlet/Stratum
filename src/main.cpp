#include <iostream>
#include <memory>
#include "nano_cache_sim/cache_sim.hpp"

using namespace nano_cache_sim;

int main() {
  using MemType = MainMemory<"MainMemory">;
  using L3Type = Cache<"L3", MemType, 8192, 16, 64, LRUPolicy, 20>;
  using L2Type = Cache<"L2", L3Type, 512, 8, 64, LRUPolicy, 10>;
  using L1Type = Cache<"L1", L2Type, 64, 8, 64, LRUPolicy, 4>;

  std::cout << "Constructing Cache Hierarchy...\n";
  auto cache_system = std::make_unique<L1Type>(100);
  std::cout << "Starting Simulation...\n";

  // Test Allocations
  uint64_t addrA = 0x1000;
  auto res1 = cache_system->Load(addrA);
  std::cout << "Load A: Level=" << res1.hit_level
            << " TotalCycles=" << res1.total_cycles << "\n";

  uint64_t addrB = 0x2000;
  auto res2 = cache_system->Load(addrB);
  std::cout << "Load B: Level=" << res2.hit_level
            << " TotalCycles=" << res2.total_cycles << "\n";

  auto res3 = cache_system->Load(addrA);
  std::cout << "Load A: Level=" << res3.hit_level
            << " TotalCycles=" << res3.total_cycles << "\n";

  // Print Stats (Top-Down traversal needed or we just print what we have)
  // cache_system is L1
  cache_system->PrintStats();

  // To print L2 and L3, we technically need access.
  // Since unique_ptr moved ownership, we can access via GetNext().
  // L1 owns L2, L2 owns L3.
  if (auto* l2_ptr = cache_system->GetNext()) {
    l2_ptr->PrintStats();
    if (auto* l3_ptr = l2_ptr->GetNext()) {
      l3_ptr->PrintStats();
    }
  }

  return 0;
}

