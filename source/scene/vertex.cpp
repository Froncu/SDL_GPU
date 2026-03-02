#include "vertex.hpp"

namespace fro
{
   decltype(Vertex::ATTRIBUTES) Vertex::ATTRIBUTES{
      {
         {
            .location{ 0 },
            .buffer_slot{ 0 },
            .format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4 },
            .offset{ offsetof(Vertex, color) }
         },
         {
            .location{ 1 },
            .buffer_slot{ 0 },
            .format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3 },
            .offset{ offsetof(Vertex, position) }
         },
         {
            .location{ 2 },
            .buffer_slot{ 0 },
            .format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3 },
            .offset{ offsetof(Vertex, normal) }
         },
         {
            .location{ 3 },
            .buffer_slot{ 0 },
            .format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3 },
            .offset{ offsetof(Vertex, tangent) }
         },
         {
            .location{ 4 },
            .buffer_slot{ 0 },
            .format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3 },
            .offset{ offsetof(Vertex, bitangent) }
         },
         {
            .location{ 5 },
            .buffer_slot{ 0 },
            .format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2 },
            .offset{ offsetof(Vertex, texture_coordinates) }
         }
      }
   };

   decltype(Vertex::BUFFER_DESCRIPTIONS) Vertex::BUFFER_DESCRIPTIONS{
      {
         {
            .slot{ 0 },
            .pitch{ sizeof(Vertex) },
            .input_rate{ SDL_GPU_VERTEXINPUTRATE_VERTEX }
         }
      }
   };
}