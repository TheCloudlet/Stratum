#ifndef POLICIES_HPP
#define POLICIES_HPP

#include <cstdint>
#include <random>
#include <vector>

namespace stratum {

// 1. Least Recently Used (LRU)
class LRUPolicy {
  size_t num_sets_;
  size_t num_ways_;
  // timestamp[set][way]
  std::vector<std::vector<uint64_t>> last_access_time_;
  std::vector<uint64_t> set_counters_;

 public:
  LRUPolicy(size_t sets, size_t ways) : num_sets_(sets), num_ways_(ways) {
    last_access_time_.resize(sets, std::vector<uint64_t>(ways, 0));
    set_counters_.resize(sets, 0);
  }

  void OnHit(size_t set_idx, size_t way_idx) {
    last_access_time_[set_idx][way_idx] = ++set_counters_[set_idx];
  }

  void OnFill(size_t set_idx, size_t way_idx) {
    last_access_time_[set_idx][way_idx] = ++set_counters_[set_idx];
  }

  size_t GetVictim(size_t set_idx) const {
    size_t victim_idx = 0;
    uint64_t min_time = UINT64_MAX;
    for (size_t way_idx = 0; way_idx < num_ways_; ++way_idx) {
      if (last_access_time_[set_idx][way_idx] < min_time) {
        min_time = last_access_time_[set_idx][way_idx];
        victim_idx = way_idx;
      }
    }
    return victim_idx;
  }
};

// 2. First-In, First-Out (FIFO)
class FIFOPolicy {
  size_t num_sets_;
  size_t num_ways_;
  std::vector<size_t> next_victim_;  // Circular buffer index per set

 public:
  FIFOPolicy(size_t sets, size_t ways) : num_sets_(sets), num_ways_(ways) {
    next_victim_.resize(sets, 0);
  }

  void OnHit(size_t, size_t) {
    // FIFO ignores hits
  }

  void OnFill(size_t set, size_t) {
    // Increment circular buffer
    next_victim_[set] = (next_victim_[set] + 1) % num_ways_;
  }

  size_t GetVictim(size_t set) const { return next_victim_[set]; }
};

// 3. Random Policy
class RandomPolicy {
  size_t num_sets_;
  size_t num_ways_;
  mutable std::mt19937 search_rng_{std::random_device{}()};

 public:
  RandomPolicy(size_t sets, size_t ways) : num_sets_(sets), num_ways_(ways) {}

  void OnHit(size_t, size_t) {}
  void OnFill(size_t, size_t) {}

  size_t GetVictim(size_t) const {
    std::uniform_int_distribution<size_t> dist(0, num_ways_ - 1);
    return dist(search_rng_);
  }
};

}  // namespace stratum

#endif  // POLICIES_HPP
