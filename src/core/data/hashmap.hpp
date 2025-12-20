#ifndef SINEN_HASHMAP_HPP
#define SINEN_HASHMAP_HPP
#include <core/allocator/global_allocator.hpp>
#include <unordered_map>
namespace sinen {
// Hash map class
template <typename K, typename V, typename Hash = std::hash<K>,
          typename Equal = std::equal_to<K>>
class Hashmap : public std::pmr::unordered_map<K, V, Hash, Equal> {
public:
  Hashmap() : std::pmr::unordered_map<K, V, Hash, Equal>(gA) {}
  Hashmap(Allocator *pA) : std::pmr::unordered_map<K, V, Hash, Equal>(pA) {}
};
} // namespace sinen

#endif // !SINEN_HASHMAP_HPP
