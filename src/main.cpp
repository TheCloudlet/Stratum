#include <memory>
#include <string>
#include <vector>

#include "nano_cache_sim/cache_sim.hpp"
#include "nano_cache_sim/trace_parser.hpp"

using namespace nano_cache_sim;

void RunSimulation(const std::string& trace_name, const std::string& filepath) {
  fmt::print("\n=========================================================\n");
  fmt::print("Running Simulation: {} ({})\n", trace_name, filepath);
  fmt::print("=========================================================\n");

  auto ops = ParseTraceFile(filepath);
  if (ops.empty()) {
    fmt::print("No operations to simulate for {}\n", trace_name);
    return;
  }

  // 1. Configuration
  using MemType = MainMemory<"MainMemory">;
  using L3Type = Cache<"L3", MemType, 8192, 16, 64, LRUPolicy, 20>;
  using L2Type = Cache<"L2", L3Type, 512, 8, 64, LRUPolicy, 10>;
  using L1Type = Cache<"L1", L2Type, 64, 8, 64, LRUPolicy, 4>;

  std::vector<std::string> hierarchy = {"L1", "L2", "L3", "MainMemory"};
  auto cache_system = std::make_unique<L1Type>(100);

  // 2. Simulation Loop
  std::vector<AccessResult> history;
  std::vector<uint64_t>
      trace_addrs;  // Keep addrs separate/consistent with current print helper

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

  // 3. Print Stats
  PrintSimulationStats(history, hierarchy);
  // Limit log output for large traces
  if (history.size() <= 20) {
    PrintAccessLog(history, trace_addrs);
  } else {
    fmt::print("\n(Detailed history hidden for large trace: {} ops)\n",
               history.size());
  }
}

int main() {
  std::vector<std::pair<std::string, std::string>> traces = {
      {"Sequential", "../test/data/sequential.txt"},
      {"Random", "../test/data/random.txt"},
      {"Temporal", "../test/data/temporal.txt"},
      {"Spatial", "../test/data/spatial.txt"},
      {"LargeLoop", "../test/data/largeloop.txt"}};

  for (const auto& t : traces) {
    RunSimulation(t.first, t.second);
  }

  return 0;
}
