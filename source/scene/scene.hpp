#ifndef SCENE_HPP
#define SCENE_HPP

#include "pch.hpp"
#include "sub_mesh.hpp"
#include "utility/unique_pointer.hpp"

class Scene final
{
   public:
      Scene(SDL_GPUDevice& gpu_device, std::filesystem::path const& path);
      Scene(Scene const&) = delete;
      Scene(Scene&&) = default;

      ~Scene() = default;

      Scene& operator=(Scene const&) = delete;
      Scene& operator=(Scene&&) = default;

      [[nodiscard]] SDL_GPUBufferBinding vertex_buffer() const;
      [[nodiscard]] SDL_GPUBufferBinding index_buffer() const;
      [[nodiscard]] std::span<SubMesh const> sub_meshes() const;

   private:
      UniquePointer<SDL_GPUBuffer> vertex_buffer_{};
      UniquePointer<SDL_GPUBuffer> index_buffer_{};
      std::vector<SubMesh> sub_meshes_{};
};

#endif //SCENE_HPP