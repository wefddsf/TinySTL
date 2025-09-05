#ifndef TINYSTL_CONSTRUCT_H_
#define TINYSTL_CONSTRUCT_H_

#include <new>

#include "type_traits.h"
#include "iterator.h"

namespace tinystl {

template <class Ty>
void construct(Ty* ptr) {
  ::new ((void*)ptr) Ty();
}

template <class Ty1, class Ty2>
void construct(Ty1* ptr, const Ty2* value) {
  ::new ((void*)ptr) Ty1(value);
}

template <class Ty1, class... Args>
void construct(Ty1* ptr, Args&&... args) {
  ::new ((void*)ptr) Ty1(tinystl::forward<Args>(args)...)
}

// 这个true_type用来接收类型萃取之后的结果的，如果是true调用true_type版本的destroy_one，反之
// 调用false_type版本的destroy_one
template <class Ty>
void destroy_one(Ty*, std::true_type) {}

template <class Ty>
void destroy_one(Ty* pointer, std::false_type) {
  if (pointer != nullptr) 
    pointer->~Ty();
}

template <class ForwardIter>
void destroy_cat(ForwardIter, ForwardIter, std::true_type) {}

template <class ForwardIter>
void destroy_cat(ForwardIter first, ForwardIter last, std::false_type) {
  for (; first != last; ++first) 
    destroy(&*first); //所以destroy是输入指针类型的
}

// 下面两个才是对外用的
template <class Ty>
void destroy(Ty* pointer) {
  destroy_one(pointer, std::is_trivially_destructible<Ty>{});
}

// 这里使用typename是因为value_type是iterator_traits的嵌套类型，cpp规定了必须用typename声明，编译器
// 才能识别出value_type是一个类型而不是一个值
template <class ForwardIter>
void destroy(ForwardIter first, ForwardIter last) {
  destroy_cat(first, last, 
              std::is_trivially_destructible<typename iterator_traits<ForwardIter>::value_type>{});
}

}


#endif