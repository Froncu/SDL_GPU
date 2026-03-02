#ifndef SUB_MESH_HPP
#define SUB_MESH_HPP

#include "pch.hpp"

namespace fro
{
   struct SubMesh final
   {
      std::int32_t vertex_offset;
      std::uint32_t index_offset;
      std::uint32_t index_count;
   };
}

#endif //SUB_MESH_HPP