#include "webgpu_command_buffer.hpp"

#include "webgpu_convert.hpp"
#include "webgpu_copy_pass.hpp"
#include "webgpu_compute_pipeline.hpp"
#include "webgpu_buffer.hpp"
#include "webgpu_device.hpp"
#include "webgpu_render_pass.hpp"
#include "webgpu_texture.hpp"
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
  for (auto buffer : retainedUniformBuffers) {
    if (buffer) {
      wgpuBufferDestroy(buffer);
      wgpuBufferRelease(buffer);
    }
  }
  retainedUniformBuffers.clear();

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

  for (auto &[slot, binding] : computeUniformBindings) {
    (void)slot;
    if (binding.buffer) {
      wgpuBufferDestroy(binding.buffer);
      wgpuBufferRelease(binding.buffer);
      binding.buffer = nullptr;
    }
  }
  computeUniformBindings.clear();
}

void CommandBuffer::clearDrawBindings() {
  for (auto &[slot, binding] : vertexUniformBindings) {
    (void)slot;
    if (binding.buffer) {
      retainedUniformBuffers.push_back(binding.buffer);
      binding.buffer = nullptr;
    }
  }
  vertexUniformBindings.clear();

  for (auto &[slot, binding] : fragmentUniformBindings) {
    (void)slot;
    if (binding.buffer) {
      retainedUniformBuffers.push_back(binding.buffer);
      binding.buffer = nullptr;
    }
  }
  fragmentUniformBindings.clear();

  for (auto &[slot, binding] : computeUniformBindings) {
    (void)slot;
    if (binding.buffer) {
      retainedUniformBuffers.push_back(binding.buffer);
      binding.buffer = nullptr;
    }
  }
  computeUniformBindings.clear();
}

void CommandBuffer::updateUniformBinding(
    std::unordered_map<UInt32, UniformBinding> &bindings, UInt32 slot,
    const void *data, Size size) {
  if (!data || size == 0 || !device) {
    return;
  }

  constexpr Size minPaddedUniformBufferSize = sizeof(float) * 16 * 6;
  const Size bufferSize =
      size < minPaddedUniformBufferSize ? minPaddedUniformBufferSize : size;

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
  desc.size = bufferSize;
  desc.mappedAtCreation = false;
  auto uniformBuffer = wgpuDeviceCreateBuffer(device->getNative(), &desc);
  if (!uniformBuffer) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to create WebGPU uniform buffer");
    return;
  }

  wgpuQueueWriteBuffer(device->getQueue(), uniformBuffer, 0, data, size);
  bindings.insert_or_assign(slot, UniformBinding{uniformBuffer, bufferSize});
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

void CommandBuffer::endCopyPass(Ptr<gpu::CopyPass> copyPass) { (void)copyPass; }

Ptr<gpu::ComputePass> CommandBuffer::beginComputePass(
    const Array<StorageTextureBinding> &storageTextures,
    const Array<StorageBufferBinding> &storageBuffers) {
  WGPUComputePassDescriptor desc{};
  auto nativePass = wgpuCommandEncoderBeginComputePass(commandEncoder, &desc);
  Array<StorageTextureBinding> textures(storageTextures,
                                        getCreateInfo().allocator);
  Array<StorageBufferBinding> buffers(storageBuffers, getCreateInfo().allocator);
  return makePtr<ComputePass>(getCreateInfo().allocator, *this, nativePass,
                              std::move(textures), std::move(buffers));
}

void CommandBuffer::endComputePass(Ptr<gpu::ComputePass> computePass) {
  auto pass = downCast<ComputePass>(computePass);
  if (pass) {
    pass->close();
  }
}

Ptr<gpu::RenderPass>
CommandBuffer::beginRenderPass(const Array<ColorTargetInfo> &infos,
                               const DepthStencilTargetInfo &depthStencilInfo,
                               float r, float g, float b, float a) {
  Array<WGPURenderPassColorAttachment> colorAttachments(
      getCreateInfo().allocator);
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
    depthStencilAttachment.depthLoadOp =
        convert::LoadOpFrom(depthStencilInfo.loadOp);
    depthStencilAttachment.depthStoreOp =
        convert::StoreOpFrom(depthStencilInfo.storeOp);
    depthStencilAttachment.depthClearValue = depthStencilInfo.clearDepth;
    depthStencilAttachment.depthReadOnly = false;
    depthStencilAttachment.stencilLoadOp = WGPULoadOp_Undefined;
    depthStencilAttachment.stencilStoreOp = WGPUStoreOp_Undefined;
    depthStencilAttachment.stencilClearValue = 0;
    depthStencilAttachment.stencilReadOnly = true;
    depthStencilAttachmentPtr = &depthStencilAttachment;
  }

  WGPURenderPassDescriptor passDesc{};
  passDesc.colorAttachmentCount = colorAttachments.size();
  passDesc.colorAttachments = colorAttachments.data();
  passDesc.depthStencilAttachment = depthStencilAttachmentPtr;
  passDesc.occlusionQuerySet = nullptr;
  passDesc.timestampWrites = nullptr;

  auto pass = wgpuCommandEncoderBeginRenderPass(commandEncoder, &passDesc);
  return makePtr<RenderPass>(getCreateInfo().allocator,
                             getCreateInfo().allocator, *this, pass,
                             std::move(transientViews));
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

void CommandBuffer::pushComputeUniformData(UInt32 slot, const void *data,
                                           Size size) {
  updateUniformBinding(computeUniformBindings, slot, data, size);
}

ComputePass::~ComputePass() {
  close();
  if (storageBindGroup) {
    wgpuBindGroupRelease(storageBindGroup);
    storageBindGroup = nullptr;
  }
  if (uniformBindGroup) {
    wgpuBindGroupRelease(uniformBindGroup);
    uniformBindGroup = nullptr;
  }
}

void ComputePass::close() {
  if (pass) {
    wgpuComputePassEncoderEnd(pass);
    wgpuComputePassEncoderRelease(pass);
    pass = nullptr;
  }
}

void ComputePass::bindComputePipeline(
    Ptr<gpu::ComputePipeline> computePipeline) {
  pipeline = computePipeline;
  auto nativePipeline = downCast<ComputePipeline>(pipeline);
  if (!nativePipeline || !pass) {
    return;
  }
  wgpuComputePassEncoderSetPipeline(pass, nativePipeline->getNative());
  bindResources();
}

void ComputePass::bindResources() {
  auto nativePipeline = downCast<ComputePipeline>(pipeline);
  if (!nativePipeline || !pass) {
    return;
  }

  if (!storageBuffers.empty()) {
    std::vector<WGPUBindGroupEntry> entries;
    entries.reserve(storageBuffers.size());
    for (UInt32 i = 0; i < storageBuffers.size(); ++i) {
      auto buffer = downCast<Buffer>(storageBuffers[i].buffer);
      if (!buffer) {
        continue;
      }
      WGPUBindGroupEntry entry{};
      entry.binding = i;
      entry.buffer = buffer->getNative();
      entry.offset = 0;
      entry.size = buffer->getCreateInfo().size;
      entries.push_back(entry);
    }
    if (!entries.empty()) {
      auto layout = wgpuComputePipelineGetBindGroupLayout(
          nativePipeline->getNative(), 0);
      WGPUBindGroupDescriptor desc{};
      desc.layout = layout;
      desc.entryCount = entries.size();
      desc.entries = entries.data();
      storageBindGroup = wgpuDeviceCreateBindGroup(
          commandBuffer.getDevice()->getNative(), &desc);
      wgpuBindGroupLayoutRelease(layout);
      if (storageBindGroup) {
        wgpuComputePassEncoderSetBindGroup(pass, 0, storageBindGroup, 0,
                                           nullptr);
      }
    }
  }

  const auto &uniforms = commandBuffer.getComputeUniformBindings();
  if (!uniforms.empty()) {
    std::vector<WGPUBindGroupEntry> entries;
    entries.reserve(uniforms.size());
    for (const auto &[slot, binding] : uniforms) {
      if (!binding.buffer) {
        continue;
      }
      WGPUBindGroupEntry entry{};
      entry.binding = slot;
      entry.buffer = binding.buffer;
      entry.offset = 0;
      entry.size = binding.size;
      entries.push_back(entry);
    }
    if (!entries.empty()) {
      auto layout = wgpuComputePipelineGetBindGroupLayout(
          nativePipeline->getNative(), 1);
      WGPUBindGroupDescriptor desc{};
      desc.layout = layout;
      desc.entryCount = entries.size();
      desc.entries = entries.data();
      uniformBindGroup = wgpuDeviceCreateBindGroup(
          commandBuffer.getDevice()->getNative(), &desc);
      wgpuBindGroupLayoutRelease(layout);
      if (uniformBindGroup) {
        wgpuComputePassEncoderSetBindGroup(pass, 1, uniformBindGroup, 0,
                                           nullptr);
      }
    }
  }
}

void ComputePass::dispatchWorkgroups(UInt32 groupCountX, UInt32 groupCountY,
                                     UInt32 groupCountZ) {
  if (!pass) {
    return;
  }
  wgpuComputePassEncoderDispatchWorkgroups(pass, groupCountX, groupCountY,
                                           groupCountZ);
}
} // namespace sinen::gpu::webgpu
