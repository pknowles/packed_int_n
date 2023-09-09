// Copyright (c) 2023 Pyarelal Knowles, MIT License

#include <bitset>
#include <gtest/gtest.h>
#include <iostream>
#include <iterator>
#include <tight_uint/tight_uint.hpp>
#include <ranges>

using namespace tight_uint;

// verify iterators
template <class iterator>
concept input_and_output_iterator =
    std::input_iterator<iterator> &&
    std::output_iterator<iterator,
                         typename std::iterator_traits<iterator>::value_type>;

// packed_uintn concepts
static_assert(std::ranges::random_access_range<vector<11>>);
static_assert(std::ranges::sized_range<vector<11>>);
static_assert(input_and_output_iterator<typename vector<11>::iterator>);

// reinterpret_packed_uintn concepts
static_assert(std::ranges::random_access_range<span<11, uint32_t>>);
static_assert(std::ranges::sized_range<span<11, uint32_t>>);
static_assert(input_and_output_iterator<typename span<11, uint32_t>::iterator>);

void printCharArrayInBinary(const uint8_t* arr, std::size_t size,
                            std::size_t bytesPerLine = 4) {
  for (std::size_t i = 0; i < size; i += bytesPerLine) {
    std::cout << std::setw(4) << std::hex << i
              << ": "; // Print byte number in hexadecimal
    for (std::size_t j = i; j < i + bytesPerLine && j < size; ++j) {
      std::bitset<8> binaryChar(arr[j]);
      std::cout << binaryChar << ' ';
    }
    std::cout << '\n';
  }
}

TEST(UnitTest, Empty) {
  vector<11> array;
  ASSERT_EQ(array.size(), 0);
  ASSERT_EQ(array.capacity(), 0);
}

TEST(UnitTest, Size) {
  vector<11> array(10u);
  ASSERT_EQ(array.size(), 10);
}

TEST(UnitTest, DefaultInit) {
  vector<11> array(1);
  ASSERT_EQ(*array.begin(), 0);
}

TEST(UnitTest, IteratorDistance) {
  vector<11> array(10);
  ASSERT_EQ(std::distance(array.begin(), array.end()), 10);
}

TEST(UnitTest, Capacity) {
  vector<8> array;
  array.reserve(128);
  ASSERT_EQ(array.size(), 0);
  ASSERT_EQ(array.capacity(), 128);
}

TEST(UnitTest, ConstructFillOne) {
  vector<11> array(1, 123U);
  ASSERT_EQ(*array.begin(), 123U);
}

TEST(UnitTest, ConstructFill) {
  vector<11> array(10, 123U);
  int        i = 0;
  for (auto value : array) {
    ASSERT_EQ(value, 123U) << "Index " << i;
    ++i;
  }
  ASSERT_EQ(i, 10);
}

TEST(UnitTest, RawData) {
  vector<8> array(128, 123U);
  ASSERT_EQ(array.size_bytes(), 128);
  ASSERT_EQ(*array.data(), 123U);
}

TEST(UnitTest, ConstructInitialier) {
  vector<11> array{0, 1, 2, 3, 4};
  ASSERT_EQ(array.size(), 5);
  auto it = array.begin();
  for (int i = 0; i < 5; ++i) {
    ASSERT_EQ(*it, i) << "Index " << i;
    ++it;
  }
}

TEST(UnitTest, ConstructCopy) {
  vector<11> array{0, 1, 2, 3, 4};
  vector<11> copy{array};
  ASSERT_EQ(copy.size(), 5);
  auto it = copy.begin();
  for (int i = 0; i < 5; ++i) {
    ASSERT_EQ(*it, i) << "Index " << i;
    ++it;
  }
}

TEST(UnitTest, ReverseIterate) {
  vector<11> array{0, 1, 2, 3, 4};
  ASSERT_EQ(array.size(), 5);
  auto it = std::reverse_iterator(array.end());
  for (int i = 0; i < 5; ++i) {
    ASSERT_EQ(*it, 4 - i) << "Index " << i;
    ++it;
  }
}

TEST(UnitTest, FromRange) {
  vector<11> array(std::views::iota(0u, 5u));
  ASSERT_EQ(array.size(), 5);
  auto it = array.begin();
  for (int i = 0; i < 5; ++i) {
    ASSERT_EQ(*it, i) << "Index " << i;
    ++it;
  }
}

TEST(UnitTest, ToRange) {
  vector<11> array(std::views::iota(0u, 5u));
  auto       view = std::views::all(array);
  ASSERT_EQ(view.size(), 5);
  for (int i = 0; i < 5; ++i) {
    ASSERT_EQ(view[i], i) << "Index " << i;
  }
}

TEST(UnitTest, OverflowWrite0) {
  vector<8, uint32_t> array(1);
  *array.begin() = 511u;
  ASSERT_EQ(*reinterpret_cast<uint32_t*>(array.data()), 255u);
}

TEST(UnitTest, OverflowWrite1) {
  vector<8, uint32_t> array(2);
  *(array.begin() + 1) = 255u;
  ASSERT_EQ(*reinterpret_cast<uint32_t*>(array.data()), 255u << 8);
}

TEST(UnitTest, OverflowRead) {
  vector<8, uint32_t> array(2);
  *reinterpret_cast<uint32_t*>(array.data()) = 511u;
  ASSERT_EQ(*array.begin(), 255u);
  ASSERT_EQ(*(array.begin() + 1), 511u >> 8);
}

TEST(UnitTest, Overflow8) {
  vector<8, uint32_t> array(std::views::iota(0u, 512u));
  // printCharArrayInBinary(array.data(), array.size_bytes());
  auto it = array.begin();
  for (uint32_t i = 0; i < 512; ++i) {
    ASSERT_EQ(*it, i & 0xffU) << "Index " << i;
    ++it;
  }
}

TEST(UnitTest, Overflow3) {
  vector<3, uint32_t> array(std::views::iota(0u, 512u));
  // printCharArrayInBinary(array.data(), array.size_bytes());
  auto it = array.begin();
  for (uint32_t i = 0; i < 512; ++i) {
    ASSERT_EQ(*it, i & 7) << "Index " << i;
    ++it;
  }
}

TEST(UnitTest, ConstArray) {
  const vector<3, uint32_t> array(std::views::iota(0u, 512u));
  auto                      it = array.begin();
  for (uint32_t i = 0; i < 512; ++i) {
    ASSERT_EQ(*it, i & 7) << "Index " << i;
    ++it;
  }
}

TEST(UnitTest, Subscript) {
  vector<11, uint32_t> array({0, 1, 2, 3, 4, 5});
  ASSERT_EQ(array[3], 3);
  const vector<11, uint32_t> constArray({0, 1, 2, 3, 4, 5});
  ASSERT_EQ(constArray[3], 3);
}

TEST(UnitTest, ReinterpretEmpty) {
  std::vector<uint32_t> memory;
  span<11, uint32_t>    array(memory);
  ASSERT_EQ(array.size(), 0);
}

TEST(UnitTest, ReinterpretSize) {
  std::vector<uint32_t> memory(703);
  span<11, uint32_t>    array(memory);
  ASSERT_EQ(array.size(), 2045);
}

TEST(UnitTest, ReinterpretRead) {
  std::vector<uint32_t> memory(1, 0xffffffffu);
  span<8, uint32_t>     array(memory);
  ASSERT_EQ(*array.begin(), 0xffu);
}

TEST(UnitTest, ReinterpretWrite) {
  std::vector<uint32_t> memory(1);
  span<11, uint32_t>    array(memory);
  *array.begin() = 2047u;
  ASSERT_EQ(memory[0], 2047u);
}

TEST(UnitTest, ReinterpretWriteOverflow) {
  std::vector<uint32_t> memory(1);
  span<11, uint32_t>    array(memory);
  *array.begin() = 4095u;
  ASSERT_EQ(memory[0], 2047u);
}

TEST(UnitTest, ReinterpretAccess) {
  std::vector<uint32_t> memory(704);
  span<11, uint32_t>    array(memory);
  ASSERT_EQ(array.size(), 2048);
  std::ranges::copy(std::views::iota(0u, 2048u), array.begin());
  uint32_t i = 0;
  for (const auto& v : array)
    ASSERT_EQ(v, i++);
}

TEST(UnitTest, ReinterpretConst) {
  const std::vector<uint32_t> memory(1, 4095u);
  auto                        array = make_tight_span<11>(memory);
  ASSERT_EQ(*array.begin(), 2047u);
}

TEST(UnitTest, ReinterpretAsConst) {
  std::vector<uint32_t>    memory(1, 4095u);
  span<11, const uint32_t> array(memory);
  ASSERT_EQ(*array.begin(), 2047u);
}

TEST(UnitTest, ReinterpretSubscript) {
  std::vector<uint32_t> memory(2, 4095u);
  std::ranges::copy(std::vector{0, 1, 2, 3, 4, 5},
                    span<11, uint32_t>(memory).begin());
  span<11, const uint32_t> array(memory);
  ASSERT_EQ(array[3], 3);
}

TEST(UnitTest, ReinterpretCopy) {
  std::vector<uint32_t>    memory(1, 4095u);
  span<11, uint32_t>       array(memory);
  span<11, const uint32_t> copy(array);
  ASSERT_EQ(copy[0], 2047u);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
