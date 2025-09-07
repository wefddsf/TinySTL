#ifndef TINYSTL_ITERATOR_H_
#define TINYSTL_ITERATOR_H_

#include <cstddef>

#include "type_traits.h"

namespace tinystl {

struct input_iterator_tag{};
struct output_iterator_tag{};
struct forward_iterator_tag : public input_iterator_tag{};
struct bidirectional_iterator_tag : public forward_iterator_tag{};
struct random_access_iterator_tag : public bidirectional_iterator_tag{};

template <class Category, class T, class Distance = ptrdiff_t,
          class Pointer = T*, class Reference = T&>
struct iterator {
  typedef Category iterator_category;
  typedef T value_type;
  typedef Distance difference_type;
  typedef Pointer pointer;
  typedef Reference reference;
};

template <class T>
struct has_iterator_cat {
private:
  // 这个NoType严格保证了NoType和char占用的内存大小是不同的
  struct NoType {
    char a[2];
  };

  template <class U>
  static constexpr NoType test(...);

  template <class U>
  static constexpr char test(typename U::iterator_category* = nullptr);

public:
  static constexpr bool value = sizeof(test<T>(nullptr)) == sizeof(char);
};

template <class Iterator, bool>
struct iterator_traits_impl {};

template <class Iterator>
struct iterator_traits_impl<Iterator, true> {
  typedef typename Iterator::iterator_category iterator_category;
  typedef typename Iterator::value_type value_type;
  typedef typename Iterator::pointer pointer;
  typedef typename Iterator::reference reference;
  typedef typename Iterator::difference_type difference_type;
};

template <class Iterator, bool>
struct iterator_traits_helper {};

template <class Iterator>
struct iterator_traits_helper<Iterator, true> : 
  public iterator_traits_impl<Iterator, 
                              std::is_convertible<typename Iterator::iterator_category, 
                                                  input_iterator_tag>::value ||
                              std::is_convertible<typename Iterator::iterator_category,
                                                  output_iterator_tag>::value> {};

/*
  只有Iterator有iterator_category类型且iterator_category类型为5种迭代器类型之一时，iterator_traits
  才可以萃取出5种类型。
*/ 
template <class Iterator>
struct iterator_traits : 
  public iterator_traits_helper<Iterator, has_iterator_cat<Iterator>::value> {};

// 指针可以看作迭代器的一种
template <class T>
struct iterator_traits<T*> {
  typedef random_access_iterator_tag iterator_category;
  typedef T value_type;
  typedef T* pointer;
  typedef T& reference;
  typedef ptrdiff_t difference;
};

template <class T>
struct iterator_traits<const T*> {
  typedef random_access_iterator_tag iterator_category;
  typedef T value_type;
  typedef const T* pointer;
  typedef const T& reference;
  typedef ptrdiff_t difference;
};

// 用来检测Iterator的iterator_category是否属于IteratorCat
template <class Iterator, class IteratorCat, 
          bool = has_iterator_cat<iterator_traits<Iterator>>::value>
struct has_iterator_cat_of : 
  public m_bool_constant<std::is_convertible<typename iterator_traits<Iterator>::iterator_category, 
                                             IteratorCat>::value> {};

/*
  这个用来作为主模板第三个参数为false的偏特化版本。当Iterator不存在有效的iterator_category的时候，
  调用该版本，防止报错。
*/ 
template <class Iterator, class IteratorCat>
struct has_iterator_cat_of<Iterator, IteratorCat, false> : public m_false_type {};

template <class Iter>
struct is_exactly_input_iterator : 
  public m_bool_constant<has_iterator_cat_of<Iter, input_iterator_tag>::value && 
                                             !has_iterator_cat_of<Iter, forward_iterator_tag>::value> {};

template <class Iter>
struct is_input_iterator :
  public has_iterator_cat_of<Iter, input_iterator_tag> {};
  
template <class Iter>
struct is_output_iterator :
  public has_iterator_cat_of<Iter, output_iterator_tag> {};

template <class Iter>
struct is_forward_iterator :
  public has_iterator_cat_of<Iter, forward_iterator_tag> {};

template <class Iter>
struct is_bidirectional_iterator :
  public has_iterator_cat_of<Iter, bidirectional_iterator_tag> {};

template <class Iter>
struct is_random_access_iterator :
  public has_iterator_cat_of<Iter, random_access_iterator_tag> {};

template <class Iterator>
struct is_iterator :
  public m_bool_constant<is_input_iterator<Iterator>::value ||
                         is_output_iterator<Iterator>::value> {};

template <class Iterator>
typename iterator_traits<Iterator>::iterator_category iterator_category(const Iterator&) {
  static_assert(is_iterator<Iterator>::value);
  return typename iterator_traits<Iterator>::iterator_category();
}

// 干啥用？
template <class Iterator>
typename iterator_traits<Iterator>::difference_type* distance_type(const Iterator&) {
  return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
}

template <class InputIterator>
typename iterator_traits<InputIterator>::difference_type 
distance_dispatch(InputIterator first, InputIterator last, input_iterator_tag) {
  typename iterator_traits<InputIterator>::difference_type n = 0;
  while (first != last) {
    ++first;
    ++n;
  }
  return n;
}

// random_access_iterator的重载，其余类型的input_iterator调用上面的模板
template <class RandomIter>
typename iterator_traits<RandomIter>::difference_type
distance_dispatch(RandomIter first, RandomIter last, random_access_iterator_tag) {
  return last - first;
}

template <class InputIterator>
typename iterator_traits<InputIterator>::difference_type
distance(InputIterator first, InputIterator last) {
  return distance_dispatch(first, last, iterator_category(first));
}

template <class InputIterator, class Distance>
void advance_dispatch(InputIterator& i, Distance n, input_iterator_tag) {
  while (n--)
    ++i;
}

template <class BidirectionalIterator, class Distance>
void advance_dispatch(BidirectionalIterator& i, Distance n, bidirectional_iterator_tag) {
  if (n >= 0) {
    while (n--)
      ++i;
  } else {
    while (n++)
      --i;
  }
}

template <class RandomIterator, class Distance>
void advance_dispatch(RandomIterator& i, Distance n, random_access_iterator_tag) {
  i += n;
}

template <class InputIterator, class Distance>
void advance(InputIterator& i, Distance n) {
  advance_dispatch(i, n, iterator_category(i));
}

template <class Iterator>
class reverse_iterator {
private:
  // 对应的正向迭代器
  Iterator current;

public:
  typedef typename iterator_traits<Iterator>::iterator_category iterator_category;
  typedef typename iterator_traits<Iterator>::value_type value_type;
  typedef typename iterator_traits<Iterator>::difference_type difference_type;
  typedef typename iterator_traits<Iterator>::pointer pointer;
  typedef typename iterator_traits<Iterator>::reference reference;

  typedef Iterator iterator_type;
  typedef reverse_iterator<Iterator> self;

public:
  // 默认构造函数
  reverse_iterator() {}
  // 参数构造函数（显式）
  explicit reverse_iterator(iterator_type i) : current(i) {}
  // 拷贝构造函数
  reverse_iterator(const self& rhs) : current(rhs.current) {}

};


}


#endif