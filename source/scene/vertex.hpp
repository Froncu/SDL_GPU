#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "pch.hpp"

struct Vertex final
{
   static std::array<SDL_GPUVertexAttribute, 6> const ATTRIBUTES;
   static std::array<SDL_GPUVertexBufferDescription, 1> const BUFFER_DESCRIPTIONS;

   glm::vec4 color;
   glm::vec3 position;
   glm::vec3 normal;
   glm::vec3 tangent;
   glm::vec3 bitangent;
   glm::vec2 texture_coordinates;
};

#endif