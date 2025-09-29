#ifndef MINIMAL_OPTIONAL_HPP
#define MINIMAL_OPTIONAL_HPP

#include <new>
#include <utility>

namespace baltazar {
namespace utils {

template <typename T> class Optional {
  alignas(T) unsigned char m_storage[sizeof(T)];
  bool m_hasValue = false;

public:
  Optional() = default;

  Optional(const T &value) {
    new (m_storage) T(value);
    m_hasValue = true;
  }

  Optional(T &&value) {
    new (m_storage) T(std::move(value));
    m_hasValue = true;
  }

  ~Optional() { reset(); }

  Optional &operator=(const T &value) {
    reset();
    new (m_storage) T(value);
    m_hasValue = true;
    return *this;
  }

  Optional &operator=(T &&value) {
    reset();
    new (m_storage) T(std::move(value));
    m_hasValue = true;
    return *this;
  }

  void reset() {
    if (m_hasValue) {
      getPointer()->~T();
      m_hasValue = false;
    }
  }

  bool has_value() const { return m_hasValue; }

  T &value() { return *getPointer(); }

  const T &value() const { return *getPointer(); }

  T &operator*() { return value(); }

  const T &operator*() const { return value(); }

  T *operator->() { return getPointer(); }

  const T *operator->() const { return getPointer(); }

private:
  T *getPointer() { return reinterpret_cast<T *>(m_storage); }

  const T *getPointer() const { return reinterpret_cast<const T *>(m_storage); }
};

} // namespace utils
} // namespace baltazar

#endif // MINIMAL_OPTIONAL_HPP
