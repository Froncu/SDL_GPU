#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

namespace fro
{
   #ifdef NDEBUG
   auto constexpr DEBUG{ false };
   #else
   auto constexpr DEBUG{ true };
   #endif
}

#endif