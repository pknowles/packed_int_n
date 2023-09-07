#include <gtest/gtest.h>
#include <packed_intn/packed_intn.hpp>
#include <iterator>
#include <ranges>
#include <iostream>
#include <bitset>

using namespace packed_intn;

static_assert(std::ranges::random_access_range<packed_uintn<11>>);
static_assert(std::ranges::sized_range<packed_uintn<11>>);

void printCharArrayInBinary(const uint8_t* arr, std::size_t size, std::size_t bytesPerLine = 4) {
    for (std::size_t i = 0; i < size; i += bytesPerLine) {
        std::cout << std::setw(4) << std::hex << i << ": "; // Print byte number in hexadecimal
        for (std::size_t j = i; j < i + bytesPerLine && j < size; ++j) {
            std::bitset<8> binaryChar(arr[j]);
            std::cout << binaryChar << ' ';
        }
        std::cout << '\n';
    }
}

TEST(UnitTest, Empty) {
    packed_uintn<11> array;
    ASSERT_EQ(array.size(), 0);
    ASSERT_EQ(array.capacity(), 0);
}

TEST(UnitTest, Size) {
    packed_uintn<11> array(10);
    ASSERT_EQ(array.size(), 10);
}

TEST(UnitTest, DefaultInit) {
    packed_uintn<11> array(1);
    ASSERT_EQ(*array.begin(), 0);
}

TEST(UnitTest, IteratorDistance) {
    packed_uintn<11> array(10);
    ASSERT_EQ(std::distance(array.begin(), array.end()), 10);
}

TEST(UnitTest, Capacity) {
    packed_uintn<8> array;
    array.reserve(128);
    ASSERT_EQ(array.size(), 0);
    ASSERT_EQ(array.capacity(), 128);
}

TEST(UnitTest, ConstructFillOne) {
    packed_uintn<11> array(1, 123U);
    ASSERT_EQ(*array.begin(), 123U);
}

TEST(UnitTest, ConstructFill) {
    packed_uintn<11> array(10, 123U);
    int i = 0;
    for (auto value : array) {
      ASSERT_EQ(value, 123U) << "Index " << i;
      ++i;
    }
    ASSERT_EQ(i, 10);
}

TEST(UnitTest, RawData) {
    packed_uintn<8> array(128, 123U);
    ASSERT_EQ(array.size_bytes(), 128);
    ASSERT_EQ(*array.data(), 123U);
}

TEST(UnitTest, ConstructInitialier) {
    packed_uintn<11> array{0, 1, 2, 3, 4};
    ASSERT_EQ(array.size(), 5);
    auto it = array.begin();
    for (int i = 0; i < 5; ++i) {
      ASSERT_EQ(*it, i) << "Index " << i;
      ++it;
    }
}

TEST(UnitTest, ReverseIterate) {
    packed_uintn<11> array{0, 1, 2, 3, 4};
    ASSERT_EQ(array.size(), 5);
    auto it = std::reverse_iterator(array.end());
    for (int i = 0; i < 5; ++i) {
      ASSERT_EQ(*it, 4 - i) << "Index " << i;
      ++it;
    }
}

TEST(UnitTest, FromRange) {
    packed_uintn<11> array(std::views::iota(0, 5));
    ASSERT_EQ(array.size(), 5);
    auto it = array.begin();
    for (int i = 0; i < 5; ++i) {
      ASSERT_EQ(*it, i) << "Index " << i;
      ++it;
    }
}

TEST(UnitTest, ToRange) {
    packed_uintn<11> array(std::views::iota(0, 5));
    auto view = std::views::all(array);
    ASSERT_EQ(view.size(), 5);
    for (int i = 0; i < 5; ++i) {
      ASSERT_EQ(view[i], i) << "Index " << i;
    }
}

TEST(UnitTest, Overflow8) {
    packed_uintn<8, uint32_t> array(std::views::iota(0, 512));
    //printCharArrayInBinary(array.data(), array.size_bytes());
    auto it = array.begin();
    for (uint32_t i = 0; i < 512; ++i) {
      ASSERT_EQ(*it, i & 0xffU) << "Index " << i;
      ++it;
    }
}

TEST(UnitTest, Overflow3) {
    packed_uintn<3, uint32_t> array(std::views::iota(0, 512));
    //printCharArrayInBinary(array.data(), array.size_bytes());
    auto it = array.begin();
    for (uint32_t i = 0; i < 512; ++i) {
      ASSERT_EQ(*it, i & 7) << "Index " << i;
      ++it;
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
