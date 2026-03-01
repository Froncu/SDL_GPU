#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "pch.hpp"

struct Vertex final
{
   static std::array<SDL_GPUVertexAttribute, 2> const ATTRIBUTES;
   static std::array<SDL_GPUVertexBufferDescription, 1> const BUFFER_DESCRIPTIONS;

   glm::vec3 position;
   glm::vec3 color;
};

#endif