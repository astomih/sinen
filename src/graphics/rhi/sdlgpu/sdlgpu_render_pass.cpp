#include "sdlgpu_render_pass.hpp"

#ifndef EMSCRIPTEN
#include "sdlgpu_buffer.hpp"
#include "sdlgpu_graphics_pipeline.hpp"
#include "sdlgpu_sampler.hpp"
#include "sdlgpu_texture.hpp"

#include <SDL3/SDL_gpu.h>

namespace sinen::rhi::sdlgpu {
void RenderPass::bindGraphicsPipeline(Ptr<rhi::GraphicsPipeline> pipeline) {
  SDL_BindGPUGraphicsPipeline(
      this->renderPass, downCast<GraphicsPipeline>(pipeline)->getNative());
}

void RenderPass::bindVertexBuffers(uint32 startSlot,
                                   const Array<BufferBinding> &bindings) {
  Array<SDL_GPUBufferBinding> bufferBindings(allocator);
  bufferBindings.resize(bindings.size());
  for (int i = 0; i < bindings.size(); ++i) {
    bufferBindings[i] = {};
    bufferBindings[i].buffer =
        downCast<Buffer>(bindings[i].buffer)->getNative();
    bufferBindings[i].offset = bindings[i].offset;
  }
  SDL_BindGPUVertexBuffers(this->renderPass, startSlot, bufferBindings.data(),
                           bufferBindings.size());
}

void RenderPass::bindIndexBuffer(const BufferBinding &binding,
                                 IndexElementSize indexElementSize) {
  SDL_GPUBufferBinding bufferBinding = {};
  bufferBinding.buffer = downCast<Buffer>(binding.buffer)->getNative();
  bufferBinding.offset = binding.offset;
  switch (indexElementSize) {
  case IndexElementSize::Uint16:
    SDL_BindGPUIndexBuffer(
        this->renderPass, &bufferBinding,
        SDL_GPUIndexElementSize::SDL_GPU_INDEXELEMENTSIZE_16BIT);
    break;
  case IndexElementSize::Uint32:
    SDL_BindGPUIndexBuffer(
        this->renderPass, &bufferBinding,
        SDL_GPUIndexElementSize::SDL_GPU_INDEXELEMENTSIZE_32BIT);
    break;
  default:
    assert(false && "Invalid index element size");
  }
}

void RenderPass::bindFragmentSamplers(
    uint32 startSlot, const Array<TextureSamplerBinding> &bindings) {
  Array<SDL_GPUTextureSamplerBinding> samplerBindings(allocator);
  samplerBindings.resize(bindings.size());
  for (int i = 0; i < samplerBindings.size(); ++i) {
    samplerBindings[i] = {};
    samplerBindings[i].sampler =
        downCast<Sampler>(bindings[i].sampler)->getNative();
    samplerBindings[i].texture =
        downCast<Texture>(bindings[i].texture)->getNative();
  }
  SDL_BindGPUFragmentSamplers(this->renderPass, startSlot,
                              samplerBindings.data(), samplerBindings.size());
}

void RenderPass::setViewport(const Viewport &viewport) {
  SDL_GPUViewport vp = {viewport.x,      viewport.y,        viewport.width,
                        viewport.height, viewport.minDepth, viewport.maxDepth};
  SDL_SetGPUViewport(this->renderPass, &vp);
}

void RenderPass::setScissor(int32 x, int32 y, int32 width, int32 height) {
  SDL_Rect rect = {x, y, width, height};
  SDL_SetGPUScissor(this->renderPass, &rect);
}

void RenderPass::drawPrimitives(uint32 vertexCount, uint32 instanceCount,
                                uint32 firstVertex, uint32 firstInstance) {
  SDL_DrawGPUPrimitives(this->renderPass, vertexCount, instanceCount,
                        firstVertex, firstInstance);
}

void RenderPass::drawIndexedPrimitives(uint32 indexCount, uint32 instanceCount,
                                       uint32 firstIndex, uint32 vertexOffset,
                                       uint32 firstInstance) {
  SDL_DrawGPUIndexedPrimitives(this->renderPass, indexCount, instanceCount,
                               firstIndex, vertexOffset, firstInstance);
}
} // namespace sinen::rhi::sdlgpu
#endif // EMSCRIPTEN
