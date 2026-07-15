#ifndef SINEN_ALLOCATOR_HPP
#define SINEN_ALLOCATOR_HPP
#include <memory_resource>
namespace sinen {
using Allocator = std::pmr::memory_resource;
}

#endif // !SINEN_ALLOCATOR_HPP