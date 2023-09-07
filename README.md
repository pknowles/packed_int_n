# packed_int_n
C++ standard library compatible tightly packed n-bit integer arrays and views

```
packed_uintn<11, uint32_t> array(std::views::iota(0, 2048));

printf("%u\n", array[123]);

for (auto v : array)
    printf("%u\n", v);

std::ranges::copy(array, std::ostream_iterator<uint32_t>(std::cout, ", "));
```
