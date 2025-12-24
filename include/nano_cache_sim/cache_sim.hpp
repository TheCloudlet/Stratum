#ifndef CACHE_HPP
#define CACHE_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "policies.hpp"

namespace nano_cache_sim {

struct AccessResult {
  std::string hit_level;
  size_t total_cycles;
};

enum class AccessType { kLoad, kStore };

// C++20 Fixed String for Non-Type Template Parameters
template <size_t N>
struct FixedString {
  char value[N]{};
  constexpr FixedString(const char (&str)[N]) { std::copy_n(str, N, value); }
};

// Main Memory: The bottom of the hierarchy
template <FixedString Name = "MainMemory">
class MainMemory {
  size_t latency_;

 public:
  MainMemory(size_t lat = 100) : latency_(lat) {}

  AccessResult Load(uint64_t addr) {
    return {Name.value, latency_};  // Always hits
  }

  AccessResult Store(uint64_t addr) { return {Name.value, latency_}; }
};

// Cache Template
// Usage: Cache<"L1", NextLayer, ...>
template <FixedString Name,
          typename NextLayer,  // potentially another Cache<...> or MainMemory
          size_t Sets, size_t Ways, size_t BlockSize,
          typename ReplacePolicy = LRUPolicy,
          size_t HitLatency = 1  // Default hit latency
          >
class Cache {
  struct Line {
    bool valid = false;
    bool dirty = false;
    uint64_t tag = 0;
  };

  std::unique_ptr<NextLayer> next_;  // OWNS the next layer

  // Cache State
  std::vector<std::vector<Line>> sets_;
  ReplacePolicy policy_;

  // Stats
  size_t hits_ = 0;
  size_t misses_ = 0;
  size_t evictions_ = 0;

 public:
  // Variadic Constructor: Recursively creates the next layer
  template <typename... Args>
  Cache(Args&&... args)
      : next_(std::make_unique<NextLayer>(std::forward<Args>(args)...)),
        sets_(Sets, std::vector<Line>(Ways)),
        policy_(Sets, Ways) {}

  AccessResult Load(uint64_t addr) {
    uint64_t set_idx = (addr / BlockSize) % Sets;
    uint64_t tag = (addr / BlockSize) / Sets;

    // 1. Tag Lookup
    for (size_t way_idx = 0; way_idx < Ways; ++way_idx) {
      if (sets_[set_idx][way_idx].valid && sets_[set_idx][way_idx].tag == tag) {
        // HIT
        StatsHit();
        policy_.OnHit(set_idx, way_idx);
        return {Name.value, HitLatency};
      }
    }

    // 2. MISS - Fetch from next level
    StatsMiss();
    AccessResult res = next_->Load(addr);

    // 3. Accumulate Latency
    res.total_cycles += HitLatency;

    // 4. Update Cache (fill)
    Fill(set_idx, tag);

    return res;
  }

  AccessResult Store(uint64_t addr) {
    uint64_t set_idx = (addr / BlockSize) % Sets;
    uint64_t tag = (addr / BlockSize) / Sets;

    // 1. Tag Lookup
    for (size_t way_idx = 0; way_idx < Ways; ++way_idx) {
      if (sets_[set_idx][way_idx].valid && sets_[set_idx][way_idx].tag == tag) {
        // HIT
        StatsHit();
        sets_[set_idx][way_idx].dirty = true;
        policy_.OnHit(set_idx, way_idx);
        return {Name.value, HitLatency};
      }
    }

    // 2. Write Miss -> Write Allocate
    StatsMiss();
    AccessResult res = next_->Load(addr);
    res.total_cycles += HitLatency;

    // 3. Fill and mark dirty
    Fill(set_idx, tag);

    for (size_t way_idx = 0; way_idx < Ways; ++way_idx) {
      if (sets_[set_idx][way_idx].valid && sets_[set_idx][way_idx].tag == tag) {
        sets_[set_idx][way_idx].dirty = true;
        break;
      }
    }

    return res;
  }

  // Helper to print stats
  void PrintStats() const {
    std::cout << "Cache " << Name.value << ": "
              << "Hits=" << hits_ << ", Misses=" << misses_
              << ", Evictions=" << evictions_ << "\n";
  }

  void PrintAllStats() const {
    PrintStats();
    // next_->PrintAllStats(); // Requires generic interface or SFINAE
  }

  NextLayer* GetNext() const { return next_.get(); }

 private:
  void Fill(size_t set_idx, uint64_t tag) {
    size_t victim_way_idx = Ways;
    for (size_t way_idx = 0; way_idx < Ways; ++way_idx) {
      if (!sets_[set_idx][way_idx].valid) {
        victim_way_idx = way_idx;
        break;
      }
    }

    if (victim_way_idx == Ways) {
      victim_way_idx = policy_.GetVictim(set_idx);
      Line& victim = sets_[set_idx][victim_way_idx];
      if (victim.valid && victim.dirty) {
        uint64_t evict_addr = (victim.tag * Sets + set_idx) * BlockSize;
        next_->Store(evict_addr);
        evictions_++;
      }
    }

    sets_[set_idx][victim_way_idx].valid = true;
    sets_[set_idx][victim_way_idx].tag = tag;
    sets_[set_idx][victim_way_idx].dirty = false;
    policy_.OnFill(set_idx, victim_way_idx);
  }

  void StatsHit() { hits_++; }
  void StatsMiss() { misses_++; }
};

}  // namespace nano_cache_sim

#endif  // CACHE_HPP
