#include <iostream>

using std::cout;
using std::endl;


// template<typename T, typename... Args>
// void expand(const T& func, Args&&... args) {
//   int arr[] = {(func(std::forward<Args>(args)), 0)...};
// }

int main() {
  // expand([] (int i) -> void {cout << i << endl;}, 1, 2);
  int* p = static_cast<int*>(::operator new(0));
  cout << *p << endl;
  return 0;
}
