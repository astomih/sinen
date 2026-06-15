#include "sdlgpu_command_buffer.hpp"

#ifndef EMSCRIPTEN
#include "sdlgpu_buffer.hpp"
#include "sdlgpu_compute_pipeline.hpp"
#include "sdlgpu_convert.hpp"
#include "sdlgpu_copy_pass.hpp"
#include "sdlgpu_render_pass.hpp"
#include "sdlgpu_texture.hpp"

#include <SDL3/SDL_gpu.h>

namespace sinen::gpu::sdlgpu {
Ptr<gpu::CopyPass> CommandBuffer::beginCopyPass() {
  auto *pass = SDL_BeginGPUCopyPass(this->commandBuffer);
  return makePtr<CopyPass>(getCreateInfo().allocator, getCreateInfo().allocator,
                           *this, pass);
}

void CommandBuffer::endCopyPass(Ptr<gpu::CopyPass> copyPass) {
  SDL_EndGPUCopyPass(downCast<CopyPass>(copyPass)->getNative());
}

Ptr<gpu::ComputePass> CommandBuffer::beginComputePass(
    const Array<StorageTextureBinding> &storageTextures,
    const Array<StorageBufferBinding> &storageBuffers) {
  Array<SDL_GPUStorageTextureReadWriteBinding> textureBindings(
      getCreateInfo().allocator);
  textureBindings.resize(storageTextures.size());
  for (int i = 0; i < storageTextures.size(); ++i) {
    textureBindings[i] = {};
    if (storageTextures[i].texture) {
      textureBindings[i].texture =
          downCast<Texture>(storageTextures[i].texture)->getNative();
    }
    textureBindings[i].mip_level = storageTextures[i].mipLevel;
    textureBindings[i].layer = storageTextures[i].layer;
    textureBindings[i].cycle = storageTextures[i].cycle;
  }

  Array<SDL_GPUStorageBufferReadWriteBinding> bufferBindings(
      getCreateInfo().allocator);
  bufferBindings.resize(storageBuffers.size());
  for (int i = 0; i < storageBuffers.size(); ++i) {
    bufferBindings[i] = {};
    if (storageBuffers[i].buffer) {
      bufferBindings[i].buffer =
          downCast<Buffer>(storageBuffers[i].buffer)->getNative();
    }
    bufferBindings[i].cycle = storageBuffers[i].cycle;
  }

  auto *pass = SDL_BeginGPUComputePass(
      commandBuffer, textureBindings.data(), textureBindings.size(),
      bufferBindings.data(), bufferBindings.size());
  return makePtr<ComputePass>(getCreateInfo().allocator, pass);
}

void CommandBuffer::endComputePass(Ptr<gpu::ComputePass> computePass) {
  SDL_EndGPUComputePass(downCast<ComputePass>(computePass)->getNative());
}

Ptr<gpu::RenderPass>
CommandBuffer::beginRenderPass(const Array<ColorTargetInfo> &infos,
                               const DepthStencilTargetInfo &depthStencilInfo,
                               float r, float g, float b, float a) {
  Array<SDL_GPUColorTargetInfo> colorTargetInfos(getCreateInfo().allocator);
  colorTargetInfos.resize(infos.size());
  for (int i = 0; i < infos.size(); ++i) {
    colorTargetInfos[i] = {};
    colorTargetInfos[i].texture =
        downCast<Texture>(infos[i].texture)->getNative();
    colorTargetInfos[i].load_op = convert::LoadOpFrom(infos[i].loadOp);
    colorTargetInfos[i].store_op = infos[i].resolveTexture
                                       ? SDL_GPU_STOREOP_RESOLVE_AND_STORE
                                       : convert::StoreOpFrom(infos[i].storeOp);
    colorTargetInfos[i].resolve_texture =
        infos[i].resolveTexture
            ? downCast<Texture>(infos[i].resolveTexture)->getNative()
            : nullptr;
    colorTargetInfos[i].clear_color = {r, g, b, a};
  }
  SDL_GPUDepthStencilTargetInfo depthStencilTarget{};
  if (depthStencilInfo.texture != nullptr) {
    depthStencilTarget.texture =
        downCast<Texture>(depthStencilInfo.texture)->getNative();
    depthStencilTarget.clear_depth = depthStencilInfo.clearDepth;
    depthStencilTarget.load_op = convert::LoadOpFrom(depthStencilInfo.loadOp);
    depthStencilTarget.store_op =
        convert::StoreOpFrom(depthStencilInfo.storeOp);
    depthStencilTarget.stencil_load_op =
        convert::LoadOpFrom(depthStencilInfo.stencilLoadOp);
    depthStencilTarget.stencil_store_op =
        convert::StoreOpFrom(depthStencilInfo.stencilStoreOp);
    depthStencilTarget.cycle = depthStencilInfo.cycle;
    depthStencilTarget.clear_stencil = depthStencilInfo.clearStencil;
  }

  auto *renderPass = SDL_BeginGPURenderPass(
      commandBuffer, colorTargetInfos.data(), colorTargetInfos.size(),
      depthStencilInfo.texture ? &depthStencilTarget : nullptr);
  return makePtr<RenderPass>(getCreateInfo().allocator,
                             getCreateInfo().allocator, *this, renderPass);
}

void CommandBuffer::endRenderPass(Ptr<gpu::RenderPass> renderPass) {
  SDL_EndGPURenderPass(downCast<RenderPass>(renderPass)->getNative());
}

void CommandBuffer::pushVertexUniformData(UInt32 slot, const void *data,
                                          size_t size) {
  SDL_PushGPUVertexUniformData(this->commandBuffer, slot, data, size);
}

void CommandBuffer::pushFragmentUniformData(UInt32 slot, const void *data,
                                            size_t size) {
  SDL_PushGPUFragmentUniformData(this->commandBuffer, slot, data, size);
}

void CommandBuffer::pushComputeUniformData(UInt32 slot, const void *data,
                                           size_t size) {
  SDL_PushGPUComputeUniformData(this->commandBuffer, slot, data, size);
}

void ComputePass::bindComputePipeline(
    Ptr<gpu::ComputePipeline> computePipeline) {
  SDL_BindGPUComputePipeline(
      computePass, downCast<ComputePipeline>(computePipeline)->getNative());
}

void ComputePass::dispatchWorkgroups(UInt32 groupCountX, UInt32 groupCountY,
                                     UInt32 groupCountZ) {
  SDL_DispatchGPUCompute(computePass, groupCountX, groupCountY, groupCountZ);
}
} // namespace sinen::gpu::sdlgpu
#endif // EMSCRIPTEN
