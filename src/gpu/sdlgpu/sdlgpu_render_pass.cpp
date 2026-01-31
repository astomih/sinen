#include "sdlgpu_render_pass.hpp"

#ifndef EMSCRIPTEN
#include "sdlgpu_buffer.hpp"
#include "sdlgpu_graphics_pipeline.hpp"
#include "sdlgpu_sampler.hpp"
#include "sdlgpu_texture.hpp"

#include <SDL3/SDL_gpu.h>

namespace sinen::gpu::sdlgpu {
void RenderPass::bindGraphicsPipeline(Ptr<gpu::GraphicsPipeline> pipeline) {
  SDL_BindGPUGraphicsPipeline(
      this->renderPass, downCast<GraphicsPipeline>(pipeline)->getNative());
}

void RenderPass::bindVertexBuffers(UInt32 startSlot,
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
    UInt32 startSlot, const Array<TextureSamplerBinding> &bindings) {
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
void RenderPass::bindFragmentSampler(UInt32 startSlot,
                                     const TextureSamplerBinding &binding) {
  SDL_GPUTextureSamplerBinding samplerBinding{};
  samplerBinding.sampler = downCast<Sampler>(binding.sampler)->getNative();
  samplerBinding.texture = downCast<Texture>(binding.texture)->getNative();
  SDL_BindGPUFragmentSamplers(this->renderPass, startSlot, &samplerBinding, 1);
}

void RenderPass::setViewport(const Viewport &viewport) {
  SDL_GPUViewport vp = {viewport.x,      viewport.y,        viewport.width,
                        viewport.height, viewport.minDepth, viewport.maxDepth};
  SDL_SetGPUViewport(this->renderPass, &vp);
}

void RenderPass::setScissor(Int32 x, Int32 y, Int32 width, Int32 height) {
  SDL_Rect rect = {x, y, width, height};
  SDL_SetGPUScissor(this->renderPass, &rect);
}

void RenderPass::drawPrimitives(UInt32 vertexCount, UInt32 instanceCount,
                                UInt32 firstVertex, UInt32 firstInstance) {
  SDL_DrawGPUPrimitives(this->renderPass, vertexCount, instanceCount,
                        firstVertex, firstInstance);
}

void RenderPass::drawIndexedPrimitives(UInt32 indexCount, UInt32 instanceCount,
                                       UInt32 firstIndex, UInt32 vertexOffset,
                                       UInt32 firstInstance) {
  SDL_DrawGPUIndexedPrimitives(this->renderPass, indexCount, instanceCount,
                               firstIndex, vertexOffset, firstInstance);
}
} // namespace sinen::gpu::sdlgpu
#endif // EMSCRIPTEN
