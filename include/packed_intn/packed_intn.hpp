// Copyright (c) 2023 Pyarelal Knowles, MIT License

#pragma once

#include <algorithm>
#include <iterator>
#include <limits>
#include <span>
#include <type_traits>
#include <vector>

#if __has_include(<ranges>)
#include <ranges>
#endif

namespace packed_intn {

// clang-format off
template <size_t bits> struct uint_t;
template <> struct uint_t<8>  { using type = uint8_t; };
template <> struct uint_t<16> { using type = uint16_t; };
template <> struct uint_t<32> { using type = uint32_t; };
template <> struct uint_t<64> { using type = uint64_t; };
// clang-format on

template <class iterator>
using iterator_deref_t =
    std::remove_reference_t<decltype(*std::declval<iterator>())>;

#if 0
// A class to give a value_type or const value_type depending on whether the
// iterator's dereferenced value is const or not.
template <class iterator>
using iterator_value_type =
    std::conditional<std::is_const_v<iterator_deref_t<iterator>>,
                     const typename iterator::value_type,
                     typename iterator::value_type>;
#else
template <class iterator>
using iterator_value_type = iterator_deref_t<iterator>;
#endif

template <class base_iterator, size_t bits>
class packed_uintn_value {
public:
  using value_type     = typename base_iterator::value_type;
  packed_uintn_value() = delete;
  packed_uintn_value(const packed_uintn_value& other) = delete;
  packed_uintn_value(const base_iterator& value, uint8_t offset)
      : m_value(value), m_offset(offset) {
    static_assert(bits < s_type_bits);
    static_assert(std::is_unsigned_v<value_type>,
                  "signed types are not implemented");
  }
  const packed_uintn_value& operator=(const value_type& value) const {
    // TODO: make this atomic using std::atomic_ref
    if constexpr (s_type_bits < 64) {
      using two_uints = typename uint_t<s_type_bits * 2>::type;
      static_assert(sizeof(two_uints) == sizeof(value_type) * 2);
      two_uints shifted_mask =
          ~(static_cast<two_uints>(s_mask_bits()) << m_offset);
      two_uints shifted_value = static_cast<two_uints>(s_mask_bits() & value)
                                << m_offset;
      *m_value &= static_cast<value_type>(shifted_mask);
      *m_value |= static_cast<value_type>(shifted_value);
      // Handle bits spanning the base type
      if (m_offset > s_type_bits - bits) {
        *(m_value + 1) &= static_cast<value_type>(shifted_mask >> s_type_bits);
        *(m_value + 1) |= static_cast<value_type>(shifted_value >> s_type_bits);
      }
    } else {
      const value_type masked_value = s_mask_bits() & value;
      const value_type shifted_mask = s_mask_bits() << m_offset;
      *m_value = (*m_value & ~shifted_mask) | (masked_value << m_offset);
      // Handle bits spanning the base type
      if constexpr (s_type_bits % bits != 0) {
        if (m_offset > s_type_bits - bits) {
          uint8_t          next_offset       = s_type_bits - m_offset;
          const value_type next_shifted_mask = s_mask_bits() >> next_offset;
          base_iterator    next_value        = m_value + 1;
          *next_value = (*next_value & ~next_shifted_mask) |
                        (masked_value >> next_offset);
        }
      }
    }
    return *this;
  }
  operator value_type() const {
    if constexpr (s_type_bits < 64) {
      using two_uints = typename uint_t<s_type_bits * 2>::type;
      static_assert(sizeof(two_uints) == sizeof(value_type) * 2);
      two_uints result = *m_value;
      // Handle bits spanning the base type
      if constexpr (s_type_bits % bits != 0) {
        if (m_offset > s_type_bits - bits) {
          result |= static_cast<two_uints>(*(m_value + 1)) << s_type_bits;
        }
      }
      return (result >> m_offset) & s_mask_bits();
    } else {
      value_type result = (*m_value >> m_offset) & s_mask_bits();
      // Handle bits spanning the base type
      if constexpr (s_type_bits % bits != 0) {
        if (m_offset > s_type_bits - bits) {
          uint8_t       next_offset = s_type_bits - m_offset;
          base_iterator next_value  = m_value + 1;
          result |= ((*next_value << next_offset) & s_mask_bits());
        }
      }
      return result;
    }
  }

  packed_uintn_value& operator=(const packed_uintn_value& other) {
    *this = static_cast<value_type>(other);
    return *this;
  }

protected:
  static constexpr value_type s_mask_bits() { return (1 << bits) - 1; };
  static constexpr uint8_t    s_type_bits = sizeof(value_type) * 8;
  base_iterator               m_value;
  uint8_t                     m_offset;
};

template <class base_iterator, size_t bits>
class packed_uintn_iterator : base_iterator {
public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type        = iterator_value_type<base_iterator>;
  using reference         = packed_uintn_value<base_iterator, bits>;
  using const_reference   = reference; // must be the same
  using difference_type   = typename base_iterator::difference_type;
  using offset_type       = size_t;

  packed_uintn_iterator() : m_base(), m_offsetBits(0) {}
  packed_uintn_iterator(base_iterator iter, offset_type offsetElements)
      : m_base(iter), m_offsetBits(offsetElements * bits) {}

  reference operator*() {
    return reference(m_base + base_element_offset(), base_element_remainder());
  }

  const_reference operator*() const {
    return const_reference(m_base + base_element_offset(),
                           base_element_remainder());
  }

  reference operator[](difference_type index) { return *(*this + index); }

  const_reference operator[](difference_type index) const {
    return *(*this + index);
  }

  packed_uintn_iterator& operator++() {
    m_offsetBits += bits;
    return *this;
  }

  packed_uintn_iterator operator++(int) {
    packed_uintn_iterator temp = *this;
    ++(*this);
    return temp;
  }

  packed_uintn_iterator& operator--() {
    if (m_offsetBits > bits) {
      m_offsetBits -= bits;
    } else {
      --m_base;
      m_offsetBits += s_baseBits - bits;
    }
    return *this;
  }

  packed_uintn_iterator operator--(int) {
    packed_uintn_iterator temp = *this;
    --(*this);
    return temp;
  }

  packed_uintn_iterator& operator+=(difference_type n) {
    return *this = *this + n;
  }

  packed_uintn_iterator operator+(difference_type n) const {
    packed_uintn_iterator result(*this);
    if (n >= 0) {
      result.m_offsetBits += n * bits;
    } else if (result.m_offsetBits > -n * bits) {
      result.m_offsetBits += n * bits;
    } else {
      auto base_offset = (-n + s_baseBits - 1) / s_baseBits;
      result.m_base -= base_offset;
      result.m_offsetBits += base_offset + n * bits;
    }
    return result;
  }

  friend packed_uintn_iterator operator+(difference_type              n,
                                         const packed_uintn_iterator& it) {
    return it + n;
  }

  packed_uintn_iterator& operator-=(difference_type n) { return *this += (-n); }

  packed_uintn_iterator operator-(difference_type n) const {
    return *this = *this - n;
  }

  friend packed_uintn_iterator operator-(difference_type              n,
                                         const packed_uintn_iterator& it) {
    return it - n;
  }

  difference_type operator-(const packed_uintn_iterator& other) const {
    return (std::distance(m_base, other.m_base) * s_baseBits + m_offsetBits -
            other.m_offsetBits) /
           bits;
  }

  bool operator==(const packed_uintn_iterator& other) const {
    return (other - *this) == 0;
  }

  bool operator!=(const packed_uintn_iterator& other) const {
    return !(*this == other);
  }

  bool operator<(const packed_uintn_iterator& other) const {
    return 0 < (*this - other);
  }

  bool operator<=(const packed_uintn_iterator& other) const {
    return 0 <= (*this - other);
  }

  bool operator>(const packed_uintn_iterator& other) const {
    return 0 > (*this - other);
  }

  bool operator>=(const packed_uintn_iterator& other) const {
    return 0 >= (*this - other);
  }

  static constexpr offset_type s_baseBits = sizeof(value_type) * 8;

private:
  base_iterator m_base;
  offset_type   m_offsetBits;

  inline offset_type base_element_offset() const {
    return m_offsetBits / s_baseBits;
  }
  inline uint8_t base_element_remainder() const {
    static_assert(s_baseBits <=
                  std::numeric_limits<uint8_t>::max()); // good luck
    return m_offsetBits % s_baseBits;
  };
};

// std::vector backed array
template <size_t bits, class T = uint32_t>
class packed_uintn {
public:
  using iterator =
      packed_uintn_iterator<typename std::vector<T>::iterator, bits>;
  using const_iterator = packed_uintn_iterator<
      typename std::add_const_t<std::vector<T>>::const_iterator, bits>;
  using value_type      = typename iterator::value_type;
  using reference       = typename iterator::reference;
  using const_reference = typename const_iterator::const_reference;
  using size_type       = typename iterator::offset_type;
  using difference_type = typename iterator::difference_type;

  packed_uintn() {}
  packed_uintn(const packed_uintn& other)
      : m_container(other.m_container), m_size(other.m_size) {}
  explicit packed_uintn(packed_uintn&& other)
      : m_container(std::move(other.m_container)), m_size(other.m_size) {
    other.m_size = 0;
  }
  explicit packed_uintn(size_type size)
      : m_container(required_base_elements(size)), m_size(size) {}
  explicit packed_uintn(size_type size, const value_type& init)
      : m_container(required_base_elements(size)), m_size(size) {
    std::fill(begin(), end(), init);
  }
  packed_uintn(std::initializer_list<T> init)
      : m_container(required_base_elements(init.size())), m_size(init.size()) {
    std::copy(init.begin(), init.end(), begin());
  }

#ifdef __cpp_lib_ranges
  friend packed_uintn& operator|(std::ranges::input_range auto&& range,
                                 packed_uintn&                   container) {
    auto initial_size = container.size();
    container.resize(container.size() + std::ranges::distance(range));
    std::ranges::copy(range, container.begin() + initial_size);
    return container;
  }
  packed_uintn(std::ranges::input_range auto&& range) { range | *this; }
#endif

  reference       operator[](size_type index) { return *(begin() + index); }
  const_reference operator[](size_type index) const {
    return *(begin() + index);
  }

  iterator       begin() { return iterator(m_container.begin(), 0); }
  const_iterator begin() const {
    return const_iterator(m_container.begin(), 0);
  }
  iterator       end() { return iterator(m_container.begin(), m_size); }
  const_iterator end() const {
    return const_iterator(m_container.begin(), m_size);
  }

  uint8_t* data() { return reinterpret_cast<uint8_t*>(m_container.data()); }
  const uint8_t* data() const {
    return reinterpret_cast<const uint8_t*>(m_container.data());
  }
  size_type size_bytes() const {
    return m_container.size() * sizeof(value_type);
  }
  size_type size() const { return m_size; }

  size_type capacity() const {
    return (m_container.capacity() * iterator::s_baseBits) / bits;
  }
  void resize(size_type size) {
    m_container.resize(required_base_elements(size));
    m_size = size;
  }
  void reserve(size_type capacity) {
    m_container.reserve(required_base_elements(capacity));
  }

  reference       front() { return *begin(); }
  const_reference front() const { return *begin(); }
  reference       back() { return *(end() - 1); }
  const_reference back() const { return *(end() - 1); }
  void            push_back(const value_type& value) {
    resize(m_size + 1);
    back() = value;
  }

private:
  std::vector<T>          m_container;
  size_type               m_size = 0;
  static inline size_type required_base_elements(size_type size) {
    // Round up
    return (size * bits + iterator::s_baseBits - 1) / iterator::s_baseBits;
  }
};

// view of existing data
template <size_t bits, class T>
class reinterpret_packed_uintn {
  friend class reinterpret_packed_uintn<bits, const T>;

public:
  using iterator = packed_uintn_iterator<typename std::span<T>::iterator, bits>;
  using const_iterator  = iterator;
  using value_type      = typename iterator::value_type;
  using reference       = typename iterator::reference;
  using const_reference = typename const_iterator::const_reference;
  using size_type       = typename iterator::offset_type;
  using difference_type = typename iterator::difference_type;
  using uint_bits       = std::integral_constant<size_t, bits>;

  // Allow empty view
  reinterpret_packed_uintn() {}

  // Copy constructor to handle direct copy or non-const to const
  template <class U, class = std::enable_if_t<
                         bits == U::uint_bits::value &&
                         (std::is_same_v<T, typename U::value_type> ||
                          std::is_convertible_v<typename U::value_type*, T*>)>>
  reinterpret_packed_uintn(U& other)
      : m_span(other.m_span), m_size(other.m_size) {}

  // Span from range construction
#ifdef __cpp_lib_ranges
  template <std::ranges::contiguous_range Range>
  reinterpret_packed_uintn(Range&& range)
      : m_span(std::forward<Range>(range)),
        m_size(size_from_base_elements(m_span.size())) {}
#endif

  // Pass-through span constructor
  // a common error here is copy constructing non-const from const
  template <class... Args,
            class = std::enable_if_t<!std::conjunction_v<
                std::is_same<std::decay_t<Args>, std::decay_t<Args>>...>>

            >
  reinterpret_packed_uintn(Args&&... args)
      : m_span(std::forward<Args>(args)...),
        m_size(size_from_base_elements(m_span.size())) {}

  reinterpret_packed_uintn&
  operator=(const reinterpret_packed_uintn& other) = delete;

  reference       operator[](size_type index) { return *(begin() + index); }
  const_reference operator[](size_type index) const {
    return *(begin() + index);
  }

  iterator       begin() { return iterator(m_span.begin(), 0); }
  const_iterator begin() const { return const_iterator(m_span.begin(), 0); }
  iterator       end() { return iterator(m_span.begin(), m_size); }
  const_iterator end() const { return const_iterator(m_span.begin(), m_size); }

  uint8_t*       data() { return reinterpret_cast<uint8_t*>(m_span.data()); }
  const uint8_t* data() const {
    return reinterpret_cast<const uint8_t*>(m_span.data());
  }
  size_type size_bytes() const { return m_span.size() * sizeof(value_type); }
  size_type size() const { return m_size; }

private:
  std::span<T>            m_span;
  size_type               m_size = 0;
  static inline size_type size_from_base_elements(size_type size) {
    // Round down
    return (size * iterator::s_baseBits) / bits;
  }
};

#ifdef __cpp_lib_ranges
template <size_t bits>
auto make_reinterpret_packed_uintn(auto&& range) {
  // TODO: this fails due to the reference wrapper, packed_uintn_value
  using value_type = iterator_value_type<decltype(range.begin())>;
  // using value_type = decltype(range.begin())::value_type;
  return reinterpret_packed_uintn<bits, value_type>(range);
}
#endif

} // namespace packed_intn
