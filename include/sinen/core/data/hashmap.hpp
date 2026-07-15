#ifndef SINEN_HASHMAP_HPP
#define SINEN_HASHMAP_HPP
#include <core/allocator/global_allocator.hpp>
#include <unordered_map>
namespace sinen {
// Hash map class
template <typename K, typename T, typename Hasher = std::hash<K>,
          typename Equal = std::equal_to<K>>
using Hashmap = std::pmr::unordered_map<K, T, Hasher, Equal>;
} // namespace sinen

#endif // !SINEN_HASHMAP_HPP
