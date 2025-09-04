#ifndef TINYSTL_VECTOR_H_
#define TINYSTL_VECTOR_H_

#include <initializer_list>

namespace tinystl
{

template <class T>
class vector 
{
  static_assert(!std::is_same<bool, T>::value, "vector<bool> is abandoned in TinySTL");
public:
private:
  
}

}


#endif