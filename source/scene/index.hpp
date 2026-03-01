#ifndef INDEX_HPP
#define INDEX_HPP

#include "pch.hpp"

auto constexpr INDEX_SIZE{ SDL_GPU_INDEXELEMENTSIZE_32BIT };
using Index = std::conditional_t<INDEX_SIZE == SDL_GPU_INDEXELEMENTSIZE_32BIT, std::uint32_t, std::uint16_t>;

#endif