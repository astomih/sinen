#include <gpu/compute/compute_pipeline.hpp>

#include <core/allocator/global_allocator.hpp>
#include <core/thread/load_context.hpp>
#include <graphics/graphics.hpp>

#include <functional>
#include <memory>

namespace sinen {
void ComputePipeline::setShader(const Shader &shader) {
  this->computeShader = shader;
}

void ComputePipeline::setThreadGroupSize(UInt32 x, UInt32 y, UInt32 z) {
  threadCountX = x;
  threadCountY = y;
  threadCountZ = z;
}

bool ComputePipeline::findUniformBufferSlot(StringView name,
                                            UInt32 &slot) const {
  uint32_t resolvedSlot = 0;
  if (computeShader.findUniformBufferSlot(name, resolvedSlot)) {
    slot = resolvedSlot;
    return true;
  }
  return false;
}

void ComputePipeline::build() {
  const TaskGroup group = LoadContext::current();
  group.add();

  const bool inSetup = (LoadContext::currentPtr() != nullptr);

  auto buildOrRetry = std::make_shared<std::function<void()>>();
  *buildOrRetry = [this, buildOrRetry, group]() {
    if (this->pipeline) {
      group.done();
      return;
    }

    if (!this->computeShader.getRaw()) {
      Graphics::addPreDrawFunc(*buildOrRetry);
      return;
    }

    gpu::ComputePipeline::CreateInfo pipelineInfo{};
    pipelineInfo.allocator = GlobalAllocator::get();
    pipelineInfo.computeShader = this->computeShader.getRaw();
    pipelineInfo.threadCountX = threadCountX;
    pipelineInfo.threadCountY = threadCountY;
    pipelineInfo.threadCountZ = threadCountZ;
    this->pipeline = Graphics::getDevice()->createComputePipeline(pipelineInfo);
    group.done();
  };

  if (!inSetup && this->computeShader.getRaw()) {
    (*buildOrRetry)();
    return;
  }

  Graphics::addPreDrawFunc(*buildOrRetry);
}

} // namespace sinen
