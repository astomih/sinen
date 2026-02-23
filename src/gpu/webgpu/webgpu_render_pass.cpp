#include "webgpu_render_pass.hpp"
#include "webgpu_buffer.hpp"
#include "webgpu_command_buffer.hpp"
#include "webgpu_convert.hpp"
#include "webgpu_device.hpp"
#include "webgpu_graphics_pipeline.hpp"
#include "webgpu_sampler.hpp"
#include "webgpu_texture.hpp"

#include <vector>

namespace sinen::gpu::webgpu {
RenderPass::~RenderPass() {
  close();
  for (auto view : transientViews) {
    if (view) {
      wgpuTextureViewRelease(view);
    }
  }
}

void RenderPass::close() {
  if (closed) {
    return;
  }
  if (renderPass) {
    wgpuRenderPassEncoderEnd(renderPass);
    wgpuRenderPassEncoderRelease(renderPass);
    renderPass = nullptr;
  }
  closed = true;
}

void RenderPass::bindGraphicsPipeline(Ptr<gpu::GraphicsPipeline> pipeline) {
  currentPipeline = pipeline;
  auto nativePipeline = downCast<GraphicsPipeline>(pipeline);
  if (!nativePipeline) {
    return;
  }
  wgpuRenderPassEncoderSetPipeline(renderPass, nativePipeline->getNative());
}

void RenderPass::bindVertexBuffers(UInt32 startSlot,
                                   const Array<BufferBinding> &bindings) {
  for (int i = 0; i < bindings.size(); ++i) {
    auto buffer = downCast<Buffer>(bindings[i].buffer);
    if (!buffer) {
      continue;
    }
    wgpuRenderPassEncoderSetVertexBuffer(renderPass, startSlot + i,
                                         buffer->getNative(),
                                         bindings[i].offset, WGPU_WHOLE_SIZE);
  }
}

void RenderPass::bindIndexBuffer(const BufferBinding &binding,
                                 IndexElementSize indexElementSize) {
  auto buffer = downCast<Buffer>(binding.buffer);
  if (!buffer) {
    return;
  }
  wgpuRenderPassEncoderSetIndexBuffer(
      renderPass, buffer->getNative(),
      convert::IndexFormatFrom(indexElementSize), binding.offset,
      WGPU_WHOLE_SIZE);
}

void RenderPass::bindFragmentSamplers(
    UInt32 startSlot, const Array<TextureSamplerBinding> &bindings) {
  for (int i = 0; i < bindings.size(); ++i) {
    fragmentSamplerBindings.insert_or_assign(startSlot + i, bindings[i]);
  }
}

void RenderPass::bindFragmentSampler(UInt32 startSlot,
                                     const TextureSamplerBinding &binding) {
  fragmentSamplerBindings.insert_or_assign(startSlot, binding);
}

void RenderPass::applyUniformBindings(
    UInt32 groupIndex,
    const std::unordered_map<UInt32, UniformBinding> &bindings) {
  if (bindings.empty()) {
    return;
  }
  auto nativePipeline = downCast<GraphicsPipeline>(currentPipeline);
  if (!nativePipeline) {
    return;
  }

  auto layout = wgpuRenderPipelineGetBindGroupLayout(
      nativePipeline->getNative(), groupIndex);
  if (!layout) {
    return;
  }

  std::vector<WGPUBindGroupEntry> entries;
  entries.reserve(bindings.size());
  for (const auto &[slot, binding] : bindings) {
    if (!binding.buffer || binding.size == 0) {
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
    WGPUBindGroupDescriptor desc{};
    desc.layout = layout;
    desc.entryCount = entries.size();
    desc.entries = entries.data();
    auto bindGroup = wgpuDeviceCreateBindGroup(
        commandBuffer.getDevice()->getNative(), &desc);
    if (bindGroup) {
      wgpuRenderPassEncoderSetBindGroup(renderPass, groupIndex, bindGroup, 0,
                                        nullptr);
      wgpuBindGroupRelease(bindGroup);
    }
  }
  wgpuBindGroupLayoutRelease(layout);
}

void RenderPass::applyBindings() {
  if (!currentPipeline) {
    return;
  }
  applyUniformBindings(1, commandBuffer.getVertexUniformBindings());
  applyUniformBindings(3, commandBuffer.getFragmentUniformBindings());

  if (fragmentSamplerBindings.empty()) {
    return;
  }

  auto nativePipeline = downCast<GraphicsPipeline>(currentPipeline);
  if (!nativePipeline) {
    return;
  }

  auto layout =
      wgpuRenderPipelineGetBindGroupLayout(nativePipeline->getNative(), 2);
  if (!layout) {
    return;
  }

  std::vector<WGPUBindGroupEntry> entries;
  entries.reserve(fragmentSamplerBindings.size() * 2);
  for (const auto &[slot, binding] : fragmentSamplerBindings) {
    auto sampler = downCast<Sampler>(binding.sampler);
    auto texture = downCast<Texture>(binding.texture);
    if (!sampler || !texture) {
      continue;
    }
    auto view = texture->getView();
    if (!view && texture->getNative()) {
      view = commandBuffer.getDevice()->createDefaultTextureView(
          texture->getNative());
      if (view) {
        transientViews.push_back(view);
      }
    }
    if (!view) {
      continue;
    }

    WGPUBindGroupEntry texEntry{};
    texEntry.binding = slot * 2;
    texEntry.textureView = view;
    entries.push_back(texEntry);

    WGPUBindGroupEntry samplerEntry{};
    samplerEntry.binding = slot * 2 + 1;
    samplerEntry.sampler = sampler->getNative();
    entries.push_back(samplerEntry);
  }

  if (!entries.empty()) {
    WGPUBindGroupDescriptor desc{};
    desc.layout = layout;
    desc.entryCount = entries.size();
    desc.entries = entries.data();
    auto bindGroup = wgpuDeviceCreateBindGroup(
        commandBuffer.getDevice()->getNative(), &desc);
    if (!bindGroup) {
      // Fallback for pipelines where sampled images are exposed as texture-only
      // bindings instead of split texture/sampler pairs.
      entries.clear();
      entries.reserve(fragmentSamplerBindings.size());
      for (const auto &[slot, binding] : fragmentSamplerBindings) {
        auto texture = downCast<Texture>(binding.texture);
        if (!texture) {
          continue;
        }
        auto view = texture->getView();
        if (!view && texture->getNative()) {
          view = commandBuffer.getDevice()->createDefaultTextureView(
              texture->getNative());
          if (view) {
            transientViews.push_back(view);
          }
        }
        if (!view) {
          continue;
        }
        WGPUBindGroupEntry texEntry{};
        texEntry.binding = slot;
        texEntry.textureView = view;
        entries.push_back(texEntry);
      }
      desc.entryCount = entries.size();
      desc.entries = entries.data();
      bindGroup = wgpuDeviceCreateBindGroup(
          commandBuffer.getDevice()->getNative(), &desc);
    }
    if (bindGroup) {
      wgpuRenderPassEncoderSetBindGroup(renderPass, 2, bindGroup, 0, nullptr);
      wgpuBindGroupRelease(bindGroup);
    }
  }
  wgpuBindGroupLayoutRelease(layout);
}

void RenderPass::setViewport(const Viewport &viewport) {
  wgpuRenderPassEncoderSetViewport(renderPass, viewport.x, viewport.y,
                                   viewport.width, viewport.height,
                                   viewport.minDepth, viewport.maxDepth);
}

void RenderPass::setScissor(Int32 x, Int32 y, Int32 width, Int32 height) {
  const auto sx = static_cast<UInt32>(x < 0 ? 0 : x);
  const auto sy = static_cast<UInt32>(y < 0 ? 0 : y);
  const auto sw = static_cast<UInt32>(width < 0 ? 0 : width);
  const auto sh = static_cast<UInt32>(height < 0 ? 0 : height);
  wgpuRenderPassEncoderSetScissorRect(renderPass, sx, sy, sw, sh);
}

void RenderPass::drawPrimitives(UInt32 vertexCount, UInt32 instanceCount,
                                UInt32 firstVertex, UInt32 firstInstance) {
  applyBindings();
  wgpuRenderPassEncoderDraw(renderPass, vertexCount, instanceCount, firstVertex,
                            firstInstance);
}

void RenderPass::drawIndexedPrimitives(UInt32 indexCount, UInt32 instanceCount,
                                       UInt32 firstIndex, UInt32 vertexOffset,
                                       UInt32 firstInstance) {
  applyBindings();
  wgpuRenderPassEncoderDrawIndexed(renderPass, indexCount, instanceCount,
                                   firstIndex, static_cast<Int32>(vertexOffset),
                                   firstInstance);
}
} // namespace sinen::gpu::webgpu