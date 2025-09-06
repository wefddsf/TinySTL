#include <iostream>

#include "../STL/type_traits.h"
#include "../STL/util.h"

using std::cout;
using std::endl;


int main() {
  tinystl::pair<float, float> p1 = {3.123f, 3.123f};
  tinystl::pair<int, int> p2 = std::move(p1);
  cout << p2.first << endl;
  return 0;
}



