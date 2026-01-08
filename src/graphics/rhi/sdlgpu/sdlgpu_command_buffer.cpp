#include "sdlgpu_command_buffer.hpp"

#ifndef EMSCRIPTEN
#include "sdlgpu_convert.hpp"
#include "sdlgpu_copy_pass.hpp"
#include "sdlgpu_render_pass.hpp"
#include "sdlgpu_texture.hpp"

#include <SDL3/SDL_gpu.h>

namespace sinen::rhi::sdlgpu {
Ptr<rhi::CopyPass> CommandBuffer::beginCopyPass() {
  auto *pass = SDL_BeginGPUCopyPass(this->commandBuffer);
  return makePtr<CopyPass>(getCreateInfo().allocator, getCreateInfo().allocator,
                           *this, pass);
}

void CommandBuffer::endCopyPass(Ptr<rhi::CopyPass> copyPass) {
  SDL_EndGPUCopyPass(downCast<CopyPass>(copyPass)->getNative());
}

Ptr<rhi::RenderPass>
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
    colorTargetInfos[i].store_op = convert::StoreOpFrom(infos[i].storeOp);
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

void CommandBuffer::endRenderPass(Ptr<rhi::RenderPass> renderPass) {
  SDL_EndGPURenderPass(downCast<RenderPass>(renderPass)->getNative());
}

void CommandBuffer::pushUniformData(UInt32 slot, const void *data,
                                    size_t size) {
  SDL_PushGPUVertexUniformData(this->commandBuffer, slot, data, size);
  SDL_PushGPUFragmentUniformData(this->commandBuffer, slot, data, size);
}
} // namespace sinen::rhi::sdlgpu
#endif // EMSCRIPTEN
