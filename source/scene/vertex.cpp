#include "vertex.hpp"

decltype(Vertex::ATTRIBUTES) Vertex::ATTRIBUTES{
   {
      {
         .location{ 0 },
         .buffer_slot{ 0 },
         .format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2 },
         .offset{ 0 }
      },
      {
         .location{ 1 },
         .buffer_slot{ 0 },
         .format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3 },
         .offset{ sizeof(float) * 2 }
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