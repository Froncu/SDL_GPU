#include "scene.hpp"
#include "vertex.hpp"

Scene::Scene(SDL_GPUDevice& gpu_device, std::filesystem::path const& path)
{
   if (not std::filesystem::exists(path))
      throw std::runtime_error(std::format("model file \"{}\" does not exist!", path.string()));

   Assimp::Importer importer{};
   aiScene const* const scene{
      importer.ReadFile(path.string().c_str(),
         aiProcess_Triangulate |
         aiProcess_GenSmoothNormals |
         aiProcess_CalcTangentSpace |
         aiProcess_ConvertToLeftHanded |
         aiProcess_PreTransformVertices |
         aiProcess_JoinIdenticalVertices |
         aiProcess_OptimizeMeshes |
         aiProcess_ValidateDataStructure
      )
   };

   if (not scene or scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE or not scene->mRootNode)
      throw std::runtime_error(std::format("failed to load model ({})", importer.GetErrorString()));

   std::int32_t vertex_offset{};
   std::uint32_t index_offset{};
   std::vector<Vertex> vertices{};
   std::vector<std::uint32_t> indices{};
   for (aiMesh const* const mesh : std::span{ scene->mMeshes, scene->mMeshes + scene->mNumMeshes })
   {
      std::uint32_t const vertex_count{ mesh->mNumVertices };
      vertices.reserve(vertices.size() + vertex_count);
      for (std::uint32_t index{}; index < vertex_count; ++index)
         vertices.push_back({
            .position{ mesh->mVertices[index].x, mesh->mVertices[index].y, mesh->mVertices[index].z },
            .color{ 1.0f, 0.0f, 0.0f },
            // .uv{ mesh->mTextureCoords[0][index].x, mesh->mTextureCoords[0][index].y },
            // .normal{ mesh->mNormals[index].x, mesh->mNormals[index].y, mesh->mNormals[index].z },
            // .tangent{ mesh->mTangents[index].x, mesh->mTangents[index].y, mesh->mTangents[index].z },
            // .bitangent{ mesh->mBitangents[index].x, mesh->mBitangents[index].y, mesh->mBitangents[index].z }
         });

      std::uint32_t const index_count{ mesh->mNumFaces * 3 };
      indices.reserve(indices.size() + index_count);
      for (aiFace const& face : std::span{ mesh->mFaces, mesh->mFaces + mesh->mNumFaces })
         for (std::uint32_t const index : std::span{ face.mIndices, face.mIndices + face.mNumIndices })
            indices.push_back(index);

      sub_meshes_.push_back({
         .vertex_offset{ vertex_offset },
         .index_offset{ index_offset },
         .index_count{ index_count },
         // .material_index{ mesh->mMaterialIndex }
      });

      vertex_offset += vertex_count;
      index_offset += index_count;
   }

   glm::vec3 centroid{};
   for (Vertex const& vertex : vertices)
      centroid += vertex.position;
   centroid /= static_cast<float>(vertices.size());

   for (Vertex& vertex : vertices)
      vertex.position -= centroid;

   SDL_GPUBufferCreateInfo const index_buffer_create_info{
      .usage{ SDL_GPU_BUFFERUSAGE_INDEX },
      .size{ static_cast<Uint32>(sizeof(decltype(indices)::value_type) * indices.size()) },
   };

   index_buffer_ = {
      SDL_CreateGPUBuffer(&gpu_device, &index_buffer_create_info),
      std::bind(SDL_ReleaseGPUBuffer, &gpu_device, std::placeholders::_1)
   };

   SDL_GPUTransferBufferCreateInfo const index_transfer_buffer_create_info{
      .usage{ SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD },
      .size{ static_cast<Uint32>(sizeof(decltype(indices)::value_type) * indices.size()) },
   };

   UniquePointer<SDL_GPUTransferBuffer> const index_transfer_buffer{
      SDL_CreateGPUTransferBuffer(&gpu_device, &index_transfer_buffer_create_info),
      std::bind(SDL_ReleaseGPUTransferBuffer, &gpu_device, std::placeholders::_1)
   };

   std::ranges::copy(indices,
      static_cast<decltype(indices)::value_type*>(SDL_MapGPUTransferBuffer(&gpu_device, index_transfer_buffer.get(), true)));

   SDL_UnmapGPUTransferBuffer(&gpu_device, index_transfer_buffer.get());

   SDL_GPUTransferBufferLocation const index_buffer_location{
      .transfer_buffer{ index_transfer_buffer.get() }
   };

   SDL_GPUBufferRegion const index_buffer_region{
      .buffer{ index_buffer_.get() },
      .size{ index_transfer_buffer_create_info.size }
   };

   SDL_GPUBufferCreateInfo const vertex_buffer_create_info{
      .usage{ SDL_GPU_BUFFERUSAGE_VERTEX },
      .size{ static_cast<Uint32>(sizeof(Vertex) * vertices.size()) },
   };

   vertex_buffer_ = {
      SDL_CreateGPUBuffer(&gpu_device, &vertex_buffer_create_info),
      std::bind(SDL_ReleaseGPUBuffer, &gpu_device, std::placeholders::_1)
   };

   SDL_GPUTransferBufferCreateInfo const vertex_transfer_buffer_create_info{
      .usage{ SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD },
      .size{ vertex_buffer_create_info.size },
   };

   UniquePointer<SDL_GPUTransferBuffer> const vertex_transfer_buffer{
      SDL_CreateGPUTransferBuffer(&gpu_device, &vertex_transfer_buffer_create_info),
      std::bind(SDL_ReleaseGPUTransferBuffer, &gpu_device, std::placeholders::_1)
   };

   std::ranges::copy(vertices,
      static_cast<Vertex*>(SDL_MapGPUTransferBuffer(&gpu_device, vertex_transfer_buffer.get(), true)));

   SDL_UnmapGPUTransferBuffer(&gpu_device, vertex_transfer_buffer.get());

   SDL_GPUTransferBufferLocation const vertex_buffer_location{
      .transfer_buffer{ vertex_transfer_buffer.get() }
   };

   SDL_GPUBufferRegion const vertex_buffer_region{
      .buffer{ vertex_buffer_.get() },
      .size{ vertex_buffer_create_info.size }
   };

   SDL_GPUCommandBuffer* const command_buffer{ SDL_AcquireGPUCommandBuffer(&gpu_device) };
   if (not command_buffer)
      std::abort();

   SDL_GPUCopyPass* const copy_pass{ SDL_BeginGPUCopyPass(command_buffer) };
   SDL_UploadToGPUBuffer(copy_pass, &index_buffer_location, &index_buffer_region, false);
   SDL_UploadToGPUBuffer(copy_pass, &vertex_buffer_location, &vertex_buffer_region, false);
   SDL_EndGPUCopyPass(copy_pass);

   if (not SDL_SubmitGPUCommandBuffer(command_buffer))
      std::abort();
}

SDL_GPUBufferBinding Scene::vertex_buffer() const
{
   return {
      .buffer{ vertex_buffer_.get() }
   };
}

SDL_GPUBufferBinding Scene::index_buffer() const
{
   return {
      .buffer{ index_buffer_.get() }
   };
}

std::span<SubMesh const> Scene::sub_meshes() const
{
   return sub_meshes_;
}