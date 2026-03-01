#ifndef UNIQUE_POINTER_HPP
#define UNIQUE_POINTER_HPP

#include "pch.hpp"

template<typename Type>
using UniquePointer = std::unique_ptr<Type, std::function<void(Type*)>>;

#endif