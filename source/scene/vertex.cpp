#include "vertex.hpp"

decltype(Vertex::ATTRIBUTES) Vertex::ATTRIBUTES{
   {
      {
         .location{ 0 },
         .buffer_slot{ 0 },
         .format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3 },
         .offset{ offsetof(Vertex, position) }
      },
      {
         .location{ 1 },
         .buffer_slot{ 0 },
         .format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3 },
         .offset{ offsetof(Vertex, color) }
      }
   }
};

decltype(Vertex::BUFFER_DESCRIPTIONS) Vertex::BUFFER_DESCRIPTIONS{
   {
      {
         .pitch{ sizeof(Vertex) },
         .input_rate{ SDL_GPU_VERTEXINPUTRATE_VERTEX }
      }
   }
};