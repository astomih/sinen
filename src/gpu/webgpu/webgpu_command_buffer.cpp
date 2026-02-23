#include "webgpu_command_buffer.hpp"

#ifndef EMSCRIPTEN
#include "webgpu_copy_pass.hpp"
#include "webgpu_device.hpp"
#include "webgpu_render_pass.hpp"
#include "webgpu_texture.hpp"
#include "webgpu_convert.hpp"
#include <SDL3/SDL.h>
#include <vector>

namespace sinen::gpu::webgpu {
CommandBuffer::~CommandBuffer() {
  releaseUniformBindings();
  if (commandBuffer) {
    wgpuCommandBufferRelease(commandBuffer);
    commandBuffer = nullptr;
  }
  if (commandEncoder) {
    wgpuCommandEncoderRelease(commandEncoder);
    commandEncoder = nullptr;
  }
}

void CommandBuffer::releaseUniformBindings() {
  for (auto &[slot, binding] : vertexUniformBindings) {
    (void)slot;
    if (binding.buffer) {
      wgpuBufferDestroy(binding.buffer);
      wgpuBufferRelease(binding.buffer);
      binding.buffer = nullptr;
    }
  }
  vertexUniformBindings.clear();

  for (auto &[slot, binding] : fragmentUniformBindings) {
    (void)slot;
    if (binding.buffer) {
      wgpuBufferDestroy(binding.buffer);
      wgpuBufferRelease(binding.buffer);
      binding.buffer = nullptr;
    }
  }
  fragmentUniformBindings.clear();
}

void CommandBuffer::updateUniformBinding(
    std::unordered_map<UInt32, UniformBinding> &bindings, UInt32 slot,
    const void *data, Size size) {
  if (!data || size == 0 || !device) {
    return;
  }

  auto it = bindings.find(slot);
  if (it != bindings.end()) {
    if (it->second.buffer) {
      wgpuBufferDestroy(it->second.buffer);
      wgpuBufferRelease(it->second.buffer);
    }
    bindings.erase(it);
  }

  WGPUBufferDescriptor desc{};
  desc.label = {nullptr, 0};
  desc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
  desc.size = size;
  desc.mappedAtCreation = false;
  auto uniformBuffer = wgpuDeviceCreateBuffer(device->getNative(), &desc);
  if (!uniformBuffer) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to create WebGPU uniform buffer");
    return;
  }

  wgpuQueueWriteBuffer(device->getQueue(), uniformBuffer, 0, data, size);
  bindings.insert_or_assign(slot, UniformBinding{uniformBuffer, size});
}

WGPUCommandBuffer CommandBuffer::finish() {
  if (commandBuffer) {
    return commandBuffer;
  }
  if (!commandEncoder) {
    return nullptr;
  }

  WGPUCommandBufferDescriptor desc{};
  commandBuffer = wgpuCommandEncoderFinish(commandEncoder, &desc);
  wgpuCommandEncoderRelease(commandEncoder);
  commandEncoder = nullptr;
  return commandBuffer;
}

Ptr<gpu::CopyPass> CommandBuffer::beginCopyPass() {
  return makePtr<CopyPass>(getCreateInfo().allocator, *this);
}

void CommandBuffer::endCopyPass(Ptr<gpu::CopyPass> copyPass) {
  (void)copyPass;
}

Ptr<gpu::RenderPass>
CommandBuffer::beginRenderPass(const Array<ColorTargetInfo> &infos,
                               const DepthStencilTargetInfo &depthStencilInfo,
                               float r, float g, float b, float a) {
  Array<WGPURenderPassColorAttachment> colorAttachments(getCreateInfo().allocator);
  colorAttachments.resize(infos.size());
  std::vector<WGPUTextureView> transientViews;
  transientViews.reserve(infos.size() + 1);

  for (int i = 0; i < infos.size(); ++i) {
    auto tex = downCast<Texture>(infos[i].texture);
    WGPUTextureView view = tex ? tex->getView() : nullptr;
    if (!view && tex && tex->getNative()) {
      view = device->createDefaultTextureView(tex->getNative());
      if (view) {
        transientViews.push_back(view);
      }
    }
    colorAttachments[i] = {};
    colorAttachments[i].view = view;
    colorAttachments[i].resolveTarget = nullptr;
    colorAttachments[i].depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    colorAttachments[i].loadOp = convert::LoadOpFrom(infos[i].loadOp);
    colorAttachments[i].storeOp = convert::StoreOpFrom(infos[i].storeOp);
    colorAttachments[i].clearValue = {
        .r = r,
        .g = g,
        .b = b,
        .a = a,
    };
  }

  WGPURenderPassDepthStencilAttachment depthStencilAttachment{};
  WGPURenderPassDepthStencilAttachment *depthStencilAttachmentPtr = nullptr;
  if (depthStencilInfo.texture) {
    auto depthTex = downCast<Texture>(depthStencilInfo.texture);
    WGPUTextureView depthView = depthTex ? depthTex->getView() : nullptr;
    if (!depthView && depthTex && depthTex->getNative()) {
      depthView = device->createDefaultTextureView(depthTex->getNative());
      if (depthView) {
        transientViews.push_back(depthView);
      }
    }
    depthStencilAttachment = {};
    depthStencilAttachment.view = depthView;
    depthStencilAttachment.depthLoadOp = convert::LoadOpFrom(depthStencilInfo.loadOp);
    depthStencilAttachment.depthStoreOp =
        convert::StoreOpFrom(depthStencilInfo.storeOp);
    depthStencilAttachment.depthClearValue = depthStencilInfo.clearDepth;
    depthStencilAttachment.depthReadOnly = false;
    depthStencilAttachment.stencilLoadOp =
        convert::LoadOpFrom(depthStencilInfo.stencilLoadOp);
    depthStencilAttachment.stencilStoreOp =
        convert::StoreOpFrom(depthStencilInfo.stencilStoreOp);
    depthStencilAttachment.stencilClearValue = depthStencilInfo.clearStencil;
    depthStencilAttachment.stencilReadOnly = false;
    depthStencilAttachmentPtr = &depthStencilAttachment;
  }

  WGPURenderPassDescriptor passDesc{};
  passDesc.colorAttachmentCount = colorAttachments.size();
  passDesc.colorAttachments = colorAttachments.data();
  passDesc.depthStencilAttachment = depthStencilAttachmentPtr;
  passDesc.occlusionQuerySet = nullptr;
  passDesc.timestampWrites = nullptr;

  auto pass = wgpuCommandEncoderBeginRenderPass(commandEncoder, &passDesc);
  return makePtr<RenderPass>(getCreateInfo().allocator, getCreateInfo().allocator,
                             *this, pass, std::move(transientViews));
}

void CommandBuffer::endRenderPass(Ptr<gpu::RenderPass> renderPass) {
  auto pass = downCast<RenderPass>(renderPass);
  if (pass) {
    pass->close();
  }
}

void CommandBuffer::pushVertexUniformData(UInt32 slot, const void *data,
                                          Size size) {
  updateUniformBinding(vertexUniformBindings, slot, data, size);
}

void CommandBuffer::pushFragmentUniformData(UInt32 slot, const void *data,
                                            Size size) {
  updateUniformBinding(fragmentUniformBindings, slot, data, size);
}
} // namespace sinen::gpu::webgpu

#endif // EMSCRIPTEN
