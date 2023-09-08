# Packed N-bit unsigned integer arrays
C++ standard library compatible tightly packed n-bit integer arrays and views

!! This is a new project, not particularly well tested or optimized

I threw this together before I saw https://github.com/gpakosz/PackedArray
existed. Maybe better to use something like that instead. Although that's a C
library and the whole point of this project was to be C++ iterator compatible.

# Building

This is a header-only library. There is a CMakeLists.txt file that exports the
library but it also builds tests.

# Examples

See [tests/test_main.cpp](./tests/test_main.cpp).

Vector-like objects (it currently uses std::vector internally)

```
#include <packed_intn/packed_intn.hpp>

// A tightly packed array of 11-bit uints, accessed via uint32_t
packed_uintn<11, uint32_t> array(std::views::iota(0, 2048));

// Access operators
printf("%u\n", array[123]);

// Yes, tightly packed
std::cout << array.size_bytes() << std::endl; // 2816 = 11 * 2048 / 8
std::cout << std::vector<uint32_t>(2048).size() * sizeof(uint32_t)
          << std::endl; // 8192 = 32 * 2048 / 8

// Supports iteration
for (auto v : array)
    printf("%u\n", v);

std::ranges::copy(array, std::ostream_iterator<uint32_t>(std::cout, ", "));
```

Reinterpret existing data in case you need a view of packed values

```
// Source data. Don't free/resize while the view exists.
const std::vector<uint32_t> memory(10, 0xffffffffu);

// Reinterpret as array of packed 11-bit integers
reinterpret_packed_uintn<11, const uint32_t> array(memory);

for (const auto& v : array)
    printf("%u\n", v);
```

# Limitations

- Performance is still ~2x worse than writing some simple C functions
- It is not thread safe yet - I plan to use std::atomic_ref to fix that
- Dereferencing an iterator actually returns a reference wrapper to support writes

  ```for (auto& v : array) ... // reference to temporary does not compile```
