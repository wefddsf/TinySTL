#ifndef TINYSTL_UTIL_H_
#define TINYSTL_UITL_H_

#include <cstddef>
#include <iostream>

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

template <class Tp>
void swap(Tp& lhs, Tp& rhs) {
  auto tmp = tinystl::move(lhs);
  lhs = tinystl::move(rhs);
  rhs = tinystl::move(tmp);
}

template <class ForwardIter1, class ForwardIter2>
ForwardIter2 swap_range(ForwardIter1 first1, ForwardIter1 last1, ForwardIter2 first2) {
  for (; first1 != last1; ++first1, ++first2)
    tinystl::swap(*first1, *first2);
  return first2;
}

// 用于交换C语言风格数组
template <class Tp, size_t N>
void swap(Tp(&a)[N], Tp(&b)[N]) {
  tinystl::swap_range(a, a + N, b);
}


/*
  pair的构造函数会分为隐式构造和显式构造两种类型，主要是为了继承初始化列表内部参数类型到pair数据成员类型的
  可转换性。
  1 默认构造函数：见下文
  2 带参构造函数：2.1 相同类型的常量引用：不知道有什么用
                 2.2 不同类型的转发引用：把外部数据完美转发给内部成员的构造函数
  3 拷贝构造函数：3.1 相同内部数据成员类型：编译器默认实现
                 3.2 不同内部数据成员类型：库实现，作了隐式构造和显示构造区分
  4 移动构造函数：4.1 相同内部数据成员类型：编译器默认实现
                 4.2 不同内部数据成员类型：库实现，使用了完美转发机制，且作了隐式和显式构造区分
*/
template <class Ty1, class Ty2>
struct pair {
  typedef Ty1 first_type;
  typedef Ty2 second_type;

  first_type first;
  second_type second;
  
  // 利用了SFINAE机制，通过enable_if来判断Other1和Other2类型是否可默认构造
  // 这里Other和Ty是相等的，只是为了和下面构造函数形式上的对称引入了Other
  // 默认构造函数
  template <class Other1 = Ty1, class Other2 = Ty2,
            typename = typename std::enable_if<std::is_default_constructible<Other1>::value &&
                                               std::is_default_constructible<Other2>::value, 
                                               void>::type>
  constexpr pair() : first(), second() {}

  template <class U1 = Ty1, class U2 = Ty2, 
            typename std::enable_if<std::is_copy_constructible<U1>::value &&
                                    std::is_copy_constructible<U2>::value &&
                                    std::is_convertible<const U1&, U1>::value &&
                                    std::is_convertible<const U2&, U2>::value,
                                    int>::type = 0>
  constexpr pair(const Ty1& a, const Ty2& b) : first(a), second(b) {}

  template <class U1 = Ty1, class U2 = Ty2,
            typename std::enable_if<std::is_copy_constructible<U1>::value &&
                                    std::is_copy_constructible<U2>::value &&
                                    (!std::is_convertible<const U1&, U1>::value ||
                                    !std::is_convertible<const U2&, U2>::value),
                                    int>::type = 0>
  explicit constexpr pair(const Ty1& a, const Ty2& b) : first(a), second(b) {}

  pair(const pair& rhs) = default;
  pair(pair&& rhs) = default;

  // 利用了完美转发机制，把接收到的a和b完美转发给first和second的构造函数
  /* 这里的两个is_convertible可以理解为可转换性继承。如果Other1，2到Ty1，2的转换都是允许的话，那
  {Other1, Other2}到pair{Ty1, Ty2}的隐式类型转换也是允许的。如果Other1，2到Ty1，2的转换有一个不
  允许，那么{Other1, Other2}到pair{Ty1, Ty2}的隐式类型转换是不允许的 */
  template <class Other1, class Other2,
            typename std::enable_if<std::is_constructible<Ty1, Other1>::value &&
                                    std::is_constructible<Ty2, Other2>::value &&
                                    std::is_convertible<Other1, Ty1>::value &&
                                    std::is_convertible<Other2, Ty2>::value,
                                    int>::type = 0>
  constexpr pair(Other1&& a, Other2&& b) : first(tinystl::forward<Other1>(a)),
                                           second(tinystl::forward<Other2>(b)) {}
  
  template <class Other1, class Other2,
            typename std::enable_if<std::is_constructible<Ty1, Other1>::value &&
                                    std::is_constructible<Ty2, Other2>::value &&
                                    (!std::is_convertible<Other1, Ty1>::value ||
                                    !std::is_convertible<Other2, Ty2>::value),
                                    int>::type = 0>
  explicit constexpr pair(Other1&& a, Other2&& b) : first(tinystl::forward<Other1>(a)),
                                                    second(tinystl::forward<Other2>(b)) {}      

  template <class Other1, class Other2,
            typename std::enable_if<std::is_constructible<Ty1, const Other1&>::value &&
                                    std::is_constructible<Ty2, const Other2&>::value &&
                                    std::is_convertible<const Other1&, Ty1>::value &&
                                    std::is_convertible<const Other2&, Ty2>::value,
                                    int>::type = 0>                 
  constexpr pair(const pair<Other1, Other2>& other) : first(other.first),
                                                      second(other.second) {}

  template <class Other1, class Other2,
            typename std::enable_if<std::is_constructible<Ty1, const Other1&>::value &&
                                    std::is_constructible<Ty2, const Other2&>::value &&
                                    (!std::is_convertible<const Other1&, Ty1>::value ||
                                    !std::is_convertible<const Other2&, Ty2>::value),
                                    int>::type = 0>                 
  explicit constexpr pair(const pair<Other1, Other2>& other) : first(other.first),
                                                               second(other.second) {}

  template <class Other1, class Other2,
            typename std::enable_if<std::is_constructible<Ty1, Other1>::value &&
                                    std::is_constructible<Ty2, Other2>::value &&
                                    std::is_convertible<Other1, Ty1>::value &&
                                    std::is_convertible<Other2, Ty2>::value,
                                    int>::type = 0>                 
  constexpr pair(pair<Other1, Other2>&& other) : first(std::forward<Other1>(other.first)),
                                                 second(std::forward<Other2>(other.second)) {}

  template <class Other1, class Other2,
            typename std::enable_if<std::is_constructible<Ty1, Other1>::value &&
                                    std::is_constructible<Ty2, Other2>::value &&
                                    (!std::is_convertible<Other1, Ty1>::value ||
                                    !std::is_convertible<Other2, Ty2>::value),
                                    int>::type = 0>                 
  explicit constexpr pair(pair<Other1, Other2>&& other) : first(std::forward<Other1>(other.first)),
                                                          second(std::forward<Other2>(other.second)) {}

  pair& operator=(const pair& rhs) {
    if (this != &rhs) {
      first = rhs.first;
      second = rhs.second;
    }
    return *this;
  }

  pair& operator=(pair&& rhs) {
    if (this != &rhs) {
      first = tinystl::move(rhs.first);
      second = tinystl::move(rhs.second);
    }
    return *this;
  }

  template <class Other1, class Other2>
  pair& operator=(const pair<Other1, Other2>& other) {
    first = other.first;
    second = other.second;
    return *this;
  }

  // 完美转发
  template <class Other1, class Other2>
  pair& operator=(pair<Other1, Other2>&& other) {
    first = tinystl::forward<Other1>(other.first);
    second = tinystl::forward<Other2>(other.second);
    return *this;
  }

  ~pair() = default;

  void swap(pair& other) {
    if (this != &other) {
      tinystl::swap(first, other.first);
      tinystl::swap(second, other.second);
    }
  }
};

template <class Ty1, class Ty2>
bool operator==(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs) {
  return lhs.first == rhs.first && lhs.second == rhs.second;
}

template <class Ty1, class Ty2>
bool operator<(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs) {
  return lhs.first < rhs.first || (lhs.first == rhs.first && lhs.second < rhs.second);
}

template <class Ty1, class Ty2>
bool operator!=(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs) {
  return !(lhs == rhs);
}

template <class Ty1, class Ty2>
bool operator>(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs) {
  return rhs < lhs;
}

template <class Ty1, class Ty2>
bool operator<=(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs) {
  return !(lhs > rhs);
}

template <class Ty1, class Ty2>
bool operator>=(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs) {
  return !(lhs < rhs);
}

template <class Ty1, class Ty2>
void swap(pair<Ty1, Ty2>& lhs, pair<Ty1, Ty2>& rhs) {
  lhs.swap(rhs);
}

template <class Ty1, class Ty2>
pair<Ty1, Ty2> make_pair(Ty1&& first, Ty2&& second) {
  return pair<Ty1, Ty2>(tinystl::forward<Ty1>(first), tinystl::forward<Ty2>(second));
}

}

#endif