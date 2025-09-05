#ifndef TINYSTL_UTIL_H_
#define TINYSTL_UITL_H_

#include <cstddef>

#include "type_traits.h"

namespace tinystl {

// 这里的typename是内嵌类型声明
template <class T>
typename std::remove_reference<T>::type&& move(T&& arg) noexcept {
  return static_cast<typename std::remove_reference<T>::type&&>(arg);
}

// 这里T&&利用的是转发引用机制，不是说我返回值一定是一个右值引用
// 这里的forward只能返回左值引用和右值引用两种类型，不能返回右值
template <class T>
T&& forward(typename std::remove_reference<T>::type& arg) noexcept {
  return static_cast<T&&>(arg);
}

template <class T>
T&& forward(typename std::remove_reference<T>::type&& arg) noexcept {
  // 这个断言只有tinystl::forward<int&>(int)这种情况下会不通过，T为左值引用，arg为右值或右值引用，
  // 说明完美转发被错误使用了
  static_assert(!std::is_lvalue_reference<T>::value, "bad forward");
  return static_cast<T&&>(arg);
}

}

#endif