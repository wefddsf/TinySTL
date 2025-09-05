#ifndef TINYSTL_TYPE_TRAITS_H_
#define TINYSTL_TYPE_TRAITS_H_

#include <type_traits>

namespace tinystl {

template <class T, T v>
struct m_integral_constant {
  static constexpr T value = v;
};

template <bool b>
using m_bool_constant = m_integral_constant<bool, b>;

typedef m_bool_constant<true> m_true_type;
typedef m_bool_constant<false> m_false_type;

template <class T1, class T2>
struct pair;

// 这里没用用到T，说明除了底下那个版本以外，其它所有情况类似于
// is_pair<int>, is_pair<string>都会调用false_type的版本
template <class T>
struct is_pair : tinystl::m_false_type {};

template <class T1, class T2>
struct is_pair<tinystl::pair<T1, T2>> : tinystl::m_true_type {};

}


#endif