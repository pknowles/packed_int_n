// Copyright (c) 2023 Pyarelal Knowles, MIT License

#define ANKERL_NANOBENCH_IMPLEMENT
#include <algorithm>
#include <cassert>
#include <compare_nvidia_micromesh.h>
#include <gtest/gtest.h>
#include <nanobench.h>
#include <tight_uint/tight_uint.hpp>
#include <random>
#include <ranges>
#include <string>

using namespace tight_uint;
using namespace ankerl;

TEST(Benchmark, CompareMicromesh) {
  // Random input
  std::random_device                 rd;
  std::mt19937                       gen(rd());
  std::uniform_int_distribution<int> distribution(1, 100);
  vector<11>                         source0(100000);
  std::ranges::generate(source0, [&]() { return distribution(gen); });
  vector<11> source1(source0);

  uint32_t sum0 = 0;
  nanobench::Bench().minEpochTime(std::chrono::milliseconds(10)).run("sum packed_uintn<11>", [&] {
    sum0 = std::accumulate(source0.begin(), source0.end(), sum0);
    ankerl::nanobench::doNotOptimizeAway(sum0);
  });

  uint32_t sum1 = 0;
  nanobench::Bench().minEpochTime(std::chrono::milliseconds(10)).run("sum micromesh<11>", [&] {
    void*  data = source1.data();
    size_t size = source1.size();
    for (size_t i = 0; i < size; ++i)
      sum1 += packedReadR11UnormPackedAlign32(data, i);
    ankerl::nanobench::doNotOptimizeAway(sum0);
  });

  assert(sum0 == sum1);

  sum0 = 0;
  nanobench::Bench().minEpochTime(std::chrono::milliseconds(10)).run("fill packed_uintn<11>", [&] {
    // std::fill(source0.begin(), source0.end(), 2047u);
    for (size_t i = 0; i < source0.size(); ++i)
      source0[i] = 2047u;
    // sum0 = std::accumulate(source0.begin(), source0.end(), sum0);
    // ankerl::nanobench::doNotOptimizeAway(sum0);
  });

  sum1 = 0;
  nanobench::Bench().minEpochTime(std::chrono::milliseconds(10)).run("fill micromesh<11>", [&] {
    void*  data = source1.data();
    size_t size = source1.size();
    for (size_t i = 0; i < size; ++i)
      packedWriteR11UnormPackedAlign32(data, i, 2047u);
    // for (size_t i = 0; i < size; ++i)
    //   sum1 += packedReadR11UnormPackedAlign32(data, i);
    // ankerl::nanobench::doNotOptimizeAway(sum0);
  });

  assert(sum0 == sum1);
}