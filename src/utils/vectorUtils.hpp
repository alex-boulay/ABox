#include <functional>
#include <vector>

template <typename T, typename K>
std::vector<K> vectorMaping(
    std::vector<T>      in,
    std::function<K(T)> lambda
)
{
  std::vector<K> out;
  out.reserve(in.size());
  for (auto val : in) {
    out.push_back(lambda(val));
  }
  return out;
}
