// Copyright (c) 2023 Pyarelal Knowles, MIT License

#pragma once

#include <type_traits>
#include <vector>
#include <limits>
#include <iterator>

namespace packed_intn {

template <class T, size_t bits>
class packed_uintn_value {
public:
  packed_uintn_value(T& value, uint8_t offset)
      : m_value(value), m_offset(offset) {
    static_assert(bits < s_typeBits);
    static_assert(std::is_unsigned_v<T>, "signed types are not implemented");
  }
  packed_uintn_value& operator=(const T& value) {
    const T mask = s_maskBits << m_offset;

    // TODO: make this atomic using std::atomic_ref
    m_value = (m_value & ~mask) | (value << m_offset);

    // Handle bits spanning the base type
    if constexpr (s_typeBits % bits != 0) {
      uint8_t nextOffset = s_typeBits - m_offset;
      const T nextMask   = s_maskBits >> nextOffset;
      T&      nextValue  = *(&m_value + 1);
      nextValue = m_offset ? (nextValue & ~nextMask) | (value >> nextOffset)
                           : nextValue;
    }

    return *this;
  }
  operator T() const {
    T result = (m_value >> m_offset) & s_maskBits;

    // Handle bits spanning the base type
    if constexpr (s_typeBits % bits != 0) {
      uint8_t nextOffset = s_typeBits - m_offset;
      T&      nextValue  = *(&m_value + 1);
      result |= m_offset ? (nextValue << nextOffset) & s_maskBits : 0;
    }

    return result;
  }

protected:
  static constexpr T s_maskBits = (1 << bits) - 1;
  static constexpr uint8_t s_typeBits = sizeof(T) * 8;
  T&                 m_value;
  uint8_t            m_offset;
};

#if 0
template <class T>
class iterator_pointer {
public:
  iterator_pointer(const T& ref) : m_ref(ref) {}
  T&       operator*() { return m_ref; };
  const T& operator*() const { return m_ref; };
private:
  T m_ref;
};
#endif

template <typename base, size_t bits>
class packed_uintn_iterator : base {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename base::value_type;
    using reference = packed_uintn_value<value_type, bits>;
    using const_reference = packed_uintn_value<value_type, bits>; // TODO: const??
    //using pointer = iterator_pointer<reference>;
    //using const_pointer = iterator_pointer<const_reference>;
    using difference_type = typename base::difference_type;
    using offset_type = size_t;

    packed_uintn_iterator() : m_base(), m_offsetBits(0) {}
    packed_uintn_iterator(base iter, offset_type offsetElements) : m_base(iter), m_offsetBits(offsetElements * bits) {}

    reference operator*() {
        return reference(*(m_base + base_element_offset()), base_element_remainder());
    }

    const_reference operator*() const {
        return const_reference(*(m_base + base_element_offset()), base_element_remainder());
    }

    reference operator[](difference_type index) {
        return *(*this + index);
    }

    const_reference operator[](difference_type index) const {
        return *(*this + index);
    }

    /*
    pointer operator->() {
        return pointer(**this);
    }

    const_pointer operator->() const {
        return pointer(**this);
    }
    */

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

    friend packed_uintn_iterator operator+(difference_type n, const packed_uintn_iterator& it) {
        return it + n;
    }

    packed_uintn_iterator& operator-=(difference_type n) {
        return *this += (-n);
    }

    packed_uintn_iterator operator-(difference_type n) const {
        return *this = *this - n;
    }

    friend packed_uintn_iterator operator-(difference_type n, const packed_uintn_iterator& it) {
        return it - n;
    }

    difference_type operator-(const packed_uintn_iterator& other) const {
        return (std::distance(m_base, other.m_base) * s_baseBits +
                m_offsetBits - other.m_offsetBits) /
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
    base m_base;
    offset_type m_offsetBits;

    inline offset_type base_element_offset() const {
        return m_offsetBits / s_baseBits;
    }
    inline uint8_t base_element_remainder() const {
        static_assert(s_baseBits <= std::numeric_limits<uint8_t>::max()); // good luck
        return m_offsetBits - base_element_offset() * s_baseBits;
    };
};

template <size_t bits, class T = uint32_t>
class packed_uintn {
  public:
    using iterator = packed_uintn_iterator<typename std::vector<T>::iterator, bits>;
    using const_iterator = packed_uintn_iterator<typename std::vector<T>::const_iterator, bits>;
    using value_type = typename iterator::value_type;
    using reference = typename iterator::reference;
    using const_reference = typename iterator::const_reference;
    using size_type = typename iterator::offset_type;
    using difference_type = typename iterator::difference_type;

    packed_uintn() {}
    explicit packed_uintn(size_type size)
        : m_container(required_base_elements(size)), m_size(size) {
    }
    explicit packed_uintn(size_type size, const value_type& init)
        : m_container(required_base_elements(size)), m_size(size) {
        std::fill(begin(), end(), init);
    }
    packed_uintn(std::initializer_list<T> init)
        : m_container(required_base_elements(init.size())),
          m_size(init.size()) {
        std::copy(init.begin(), init.end(), begin());
    }

#ifdef __cpp_lib_ranges
    friend packed_uintn& operator|(std::ranges::input_range auto&& range, packed_uintn& container)
    {
        auto initial_size = container.size();
        container.resize(container.size() + std::ranges::distance(range));
        std::copy(std::ranges::begin(range), std::ranges::end(range), container.begin() + initial_size);
        return container;
    }
    packed_uintn(std::ranges::input_range auto&& range) {
        range | *this;
    }
#endif

    reference       operator[](size_type index) { *(begin() + index); }
    const_reference operator[](size_type index) const { *(begin() + index); }

    iterator       begin() { return iterator(m_container.begin(), 0); }
    const_iterator begin() const {
        return const_iterator(m_container.begin(), 0);
    }
    iterator       end() { return iterator(m_container.begin(), m_size); }
    const_iterator end() const {
        return const_iterator(m_container.begin(), m_size);
    }

    uint8_t* data() { return reinterpret_cast<uint8_t*>(m_container.data()); }
    uint8_t* data() const {
        return reinterpret_cast<const uint8_t*>(m_container.data());
    }
    size_type size_bytes() const { return m_container.size() * sizeof(value_type); }
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
    std::vector<T> m_container;
    size_type m_size = 0;
    static inline size_type required_base_elements(size_type size)
    {
        return (size * bits + iterator::s_baseBits - 1) /
                           iterator::s_baseBits;
    }
};

} // namespace packed_intn