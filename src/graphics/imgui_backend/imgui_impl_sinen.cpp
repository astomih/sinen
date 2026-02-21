#include "imgui_impl_sinen.hpp"
#include "imgui_impl_sinen_shaders.hpp"

namespace sinen {
struct ImGuiImplParanoixaFrameData {
  Ptr<gpu::Buffer> VertexBuffer = nullptr;
  Ptr<gpu::Buffer> IndexBuffer = nullptr;
  uint32_t VertexBufferSize = 0;
  uint32_t IndexBufferSize = 0;
};

struct ImGuiImplParanoixaData {
  ImGuiImplParanoixaInitInfo InitInfo;

  // Graphics pipeline & shaders
  Ptr<gpu::Shader> VertexShader = nullptr;
  Ptr<gpu::Shader> FragmentShader = nullptr;
  Ptr<gpu::GraphicsPipeline> Pipeline = nullptr;

  // Font data
  Ptr<gpu::Sampler> FontSampler = nullptr;
  Ptr<gpu::Texture> FontTexture = nullptr;
  gpu::TextureSamplerBinding FontBinding = {nullptr, nullptr};

  // Frame data for main window
  ImGuiImplParanoixaFrameData MainWindowFrameData;
};
static ImGuiImplParanoixaData *imGuiImplParanoixaGetBackendData() {
  return ImGui::GetCurrentContext()
             ? (ImGuiImplParanoixaData *)ImGui::GetIO().BackendRendererUserData
             : nullptr;
}
static void imguiImplParanoixaCreateShaders() {
  // Create the shader modules
  ImGuiImplParanoixaData *bd = imGuiImplParanoixaGetBackendData();
  ImGuiImplParanoixaInitInfo *v = &bd->InitInfo;

  auto driver = v->Device->getDriver();

  gpu::Shader::CreateInfo vertexShaderInfo = {};
  vertexShaderInfo.allocator = v->Allocator;
  vertexShaderInfo.entrypoint = "main";
  vertexShaderInfo.stage = gpu::ShaderStage::Vertex;
  vertexShaderInfo.numUniformBuffers = 1;
  vertexShaderInfo.numStorageBuffers = 0;
  vertexShaderInfo.numStorageTextures = 0;
  vertexShaderInfo.numSamplers = 0;

  gpu::Shader::CreateInfo fragmentShaderInfo = {};
  fragmentShaderInfo.allocator = v->Allocator;
  fragmentShaderInfo.entrypoint = "main";
  fragmentShaderInfo.stage = gpu::ShaderStage::Fragment;
  fragmentShaderInfo.numSamplers = 1;
  fragmentShaderInfo.numStorageBuffers = 0;
  fragmentShaderInfo.numStorageTextures = 0;
  fragmentShaderInfo.numUniformBuffers = 0;

  if (driver == "vulkan") {
    vertexShaderInfo.format = gpu::ShaderFormat::SPIRV;
    vertexShaderInfo.data = spirv_vertex;
    vertexShaderInfo.size = sizeof(spirv_vertex);
    fragmentShaderInfo.format = gpu::ShaderFormat::SPIRV;
    fragmentShaderInfo.data = spirv_fragment;
    fragmentShaderInfo.size = sizeof(spirv_fragment);
  }
  //   else if (strcmp(driver, "direct3d12") == 0) {
  //     vertex_shader_info.format = SDL_GPU_SHADERFORMAT_DXBC;
  //     vertex_shader_info.code = dxbc_vertex;
  //     vertex_shader_info.code_size = sizeof(dxbc_vertex);
  //     fragment_shader_info.format = SDL_GPU_SHADERFORMAT_DXBC;
  //     fragment_shader_info.code = dxbc_fragment;
  //     fragment_shader_info.code_size = sizeof(dxbc_fragment);
  //   }
  // #ifdef __APPLE__
  //   else {
  //     vertex_shader_info.entrypoint = "main0";
  //     vertex_shader_info.format = SDL_GPU_SHADERFORMAT_METALLIB;
  //     vertex_shader_info.code = metallib_vertex;
  //     vertex_shader_info.code_size = sizeof(metallib_vertex);
  //     fragment_shader_info.entrypoint = "main0";
  //     fragment_shader_info.format = SDL_GPU_SHADERFORMAT_METALLIB;
  //     fragment_shader_info.code = metallib_fragment;
  //     fragment_shader_info.code_size = sizeof(metallib_fragment);
  //   }
  // #endif
  bd->VertexShader = v->Device->createShader(vertexShaderInfo);
  bd->FragmentShader = v->Device->createShader(fragmentShaderInfo);
  IM_ASSERT(bd->VertexShader != nullptr);
  IM_ASSERT(bd->FragmentShader != nullptr);
}
static void imGuiImplParanoixaCreateGraphicsPipeline() {
  ImGuiImplParanoixaData *bd = imGuiImplParanoixaGetBackendData();
  ImGuiImplParanoixaInitInfo *v = &bd->InitInfo;
  imguiImplParanoixaCreateShaders();

  Array<gpu::VertexBufferDescription> vertexBufferDesc(v->Allocator);
  vertexBufferDesc.resize(1);
  vertexBufferDesc[0].slot = 0;
  vertexBufferDesc[0].inputRate = gpu::VertexInputRate::Vertex;
  vertexBufferDesc[0].instanceStepRate = 0;
  vertexBufferDesc[0].pitch = sizeof(ImDrawVert);

  Array<gpu::VertexAttribute> vertexAttributes(v->Allocator);
  vertexAttributes.resize(3);
  vertexAttributes[0].bufferSlot = 0;
  vertexAttributes[0].format = gpu::VertexElementFormat::Float2;
  vertexAttributes[0].location = 0;
  vertexAttributes[0].offset = offsetof(ImDrawVert, pos);

  vertexAttributes[1].bufferSlot = 0;
  vertexAttributes[1].format = gpu::VertexElementFormat::Float2;
  vertexAttributes[1].location = 1;
  vertexAttributes[1].offset = offsetof(ImDrawVert, uv);

  vertexAttributes[2].bufferSlot = 0;
  vertexAttributes[2].format = gpu::VertexElementFormat::UByte4_NORM;
  vertexAttributes[2].location = 2;
  vertexAttributes[2].offset = offsetof(ImDrawVert, col);

  gpu::VertexInputState vertexInputState{v->Allocator};
  vertexInputState.vertexAttributes = vertexAttributes;
  vertexInputState.vertexBufferDescriptions = vertexBufferDesc;

  gpu::RasterizerState rasterizerState = {};
  rasterizerState.fillMode = gpu::FillMode::Fill;
  rasterizerState.cullMode = gpu::CullMode::None;
  rasterizerState.frontFace = gpu::FrontFace::CounterClockwise;
  rasterizerState.enableDepthBias = false;
  rasterizerState.enableDepthClip = false;

  gpu::MultiSampleState multisampleState{};
  multisampleState.sampleCount = v->MSAASamples;
  multisampleState.enableMask = false;

  gpu::DepthStencilState depthStencilState = {};
  depthStencilState.enableDepthTest = false;
  depthStencilState.enableDepthWrite = false;
  depthStencilState.enableStencilTest = false;

  gpu::ColorTargetBlendState blendState = {};
  blendState.enableBlend = true;
  blendState.srcColorBlendFactor = gpu::BlendFactor::SrcAlpha;
  blendState.dstColorBlendFactor = gpu::BlendFactor::OneMinusSrcAlpha;
  blendState.colorBlendOp = gpu::BlendOp::Add;
  blendState.srcAlphaBlendFactor = gpu::BlendFactor::One;
  blendState.dstAlphaBlendFactor = gpu::BlendFactor::OneMinusSrcAlpha;
  blendState.alphaBlendOp = gpu::BlendOp::Add;
  blendState.colorWriteMask = gpu::ColorComponent::R | gpu::ColorComponent::G |
                              gpu::ColorComponent::B | gpu::ColorComponent::A;

  Array<gpu::ColorTargetDescription> colorTargetDesc(v->Allocator);
  colorTargetDesc.resize(1);
  colorTargetDesc[0].format = v->ColorTargetFormat;
  colorTargetDesc[0].blendState = blendState;

  gpu::TargetInfo targetInfo = {v->Allocator};
  targetInfo.colorTargetDescriptions = colorTargetDesc;
  targetInfo.hasDepthStencilTarget = false;

  gpu::GraphicsPipeline::CreateInfo pipelineInfo = {v->Allocator};
  pipelineInfo.allocator = v->Allocator;
  pipelineInfo.vertexShader = bd->VertexShader;
  pipelineInfo.fragmentShader = bd->FragmentShader;
  pipelineInfo.vertexInputState = vertexInputState;
  pipelineInfo.primitiveType = gpu::PrimitiveType::TriangleList;
  pipelineInfo.rasterizerState = rasterizerState;
  pipelineInfo.multiSampleState = multisampleState;
  pipelineInfo.depthStencilState = depthStencilState;
  pipelineInfo.targetInfo = targetInfo;

  bd->Pipeline = v->Device->createGraphicsPipeline(pipelineInfo);
  IM_ASSERT(bd->Pipeline != nullptr && "Failed to create graphics pipeline");
}
IMGUI_IMPL_API bool imGuiImplParanoixaInit(ImGuiImplParanoixaInitInfo *info) {
  ImGuiIO &io = ImGui::GetIO();
  IMGUI_CHECKVERSION();
  IM_ASSERT(io.BackendRendererUserData == nullptr &&
            "Already initialized a renderer backend!");

  // Setup backend capabilities flags
  ImGuiImplParanoixaData *bd = IM_NEW(ImGuiImplParanoixaData)();
  io.BackendRendererUserData = (void *)bd;
  io.BackendRendererName = "imgui_impl_paranoixa";
  io.BackendFlags |=
      ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the
                                              // ImDrawCmd::VtxOffset field,
                                              // allowing for large meshes.

  IM_ASSERT(info->Device != nullptr);
  IM_ASSERT(info->ColorTargetFormat != gpu::TextureFormat::Invalid);

  bd->InitInfo = *info;

  imGuiImplParanoixaCreateDeviceObjects();
  return true;
}

IMGUI_IMPL_API void imGuiImplParanoixaShutdown() {}

IMGUI_IMPL_API void imGuiImplParanoixaNewFrame() {
  ImGuiImplParanoixaData *bd = imGuiImplParanoixaGetBackendData();
  IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call "
                             "ImGui_ImplParanoixa_Init()?");

  if (!bd->FontTexture)
    imGuiImplParanoixaCreateFontsTexture();
}
static void createOrResizeBuffer(Ptr<gpu::Buffer> &buffer, uint32_t *old_size,
                                 uint32_t new_size, gpu::BufferUsage usage) {
  ImGuiImplParanoixaData *bd = imGuiImplParanoixaGetBackendData();
  ImGuiImplParanoixaInitInfo *v = &bd->InitInfo;

  // Even though this is fairly rarely called.
  v->Device->waitForGpuIdle();

  gpu::Buffer::CreateInfo bufferInfo = {};
  bufferInfo.allocator = v->Allocator;
  bufferInfo.usage = usage;
  bufferInfo.size = new_size;
  // buffer_info.props = 0;
  buffer = v->Device->createBuffer(bufferInfo);
  *old_size = new_size;
  IM_ASSERT(buffer != nullptr && "Failed to create GPU Buffer");
}

IMGUI_IMPL_API void
imGuiImplParanoixaPrepareDrawData(ImDrawData *draw_data,
                                  Ptr<gpu::CommandBuffer> command_buffer) {
  // Avoid rendering when minimized, scale coordinates for retina displays
  // (screen coordinates != framebuffer coordinates)
  int fbWidth = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
  int fbHeight =
      (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
  if (fbWidth <= 0 || fbHeight <= 0 || draw_data->TotalVtxCount <= 0)
    return;

  ImGuiImplParanoixaData *bd = imGuiImplParanoixaGetBackendData();
  ImGuiImplParanoixaInitInfo *v = &bd->InitInfo;
  ImGuiImplParanoixaFrameData *fd = &bd->MainWindowFrameData;

  uint32_t vertexSize = draw_data->TotalVtxCount * sizeof(ImDrawVert);
  uint32_t indexSize = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
  if (fd->VertexBuffer == nullptr || fd->VertexBufferSize < vertexSize)
    createOrResizeBuffer(fd->VertexBuffer, &fd->VertexBufferSize, vertexSize,
                         gpu::BufferUsage::Vertex);
  IM_ASSERT(fd->VertexBuffer != nullptr &&
            "Failed to create the vertex buffer");
  if (fd->IndexBuffer == nullptr || fd->IndexBufferSize < indexSize)
    createOrResizeBuffer(fd->IndexBuffer, &fd->IndexBufferSize, indexSize,
                         gpu::BufferUsage::Index);
  IM_ASSERT(fd->IndexBuffer != nullptr && "Failed to create the index buffer");

  // FIXME: It feels like more code could be shared there.
  gpu::TransferBuffer::CreateInfo vertexTransferbufferInfo = {};
  vertexTransferbufferInfo.allocator = v->Allocator;
  vertexTransferbufferInfo.usage = gpu::TransferBufferUsage::Upload;
  vertexTransferbufferInfo.size = vertexSize;
  gpu::TransferBuffer::CreateInfo indexTransferbufferInfo = {};
  indexTransferbufferInfo.allocator = v->Allocator;
  indexTransferbufferInfo.usage = gpu::TransferBufferUsage::Upload;
  indexTransferbufferInfo.size = indexSize;

  auto vertexTransferbuffer =
      v->Device->createTransferBuffer(vertexTransferbufferInfo);
  IM_ASSERT(vertexTransferbuffer != nullptr &&
            "Failed to create the vertex transfer buffer");
  auto indexTransferbuffer =
      v->Device->createTransferBuffer(indexTransferbufferInfo);
  IM_ASSERT(indexTransferbuffer != nullptr &&
            "Failed to create the index transfer buffer");

  ImDrawVert *vtxDst = (ImDrawVert *)vertexTransferbuffer->map(true);
  ImDrawIdx *idxDst = (ImDrawIdx *)indexTransferbuffer->map(true);
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const ImDrawList *drawList = draw_data->CmdLists[n];
    memcpy(vtxDst, drawList->VtxBuffer.Data,
           drawList->VtxBuffer.Size * sizeof(ImDrawVert));
    memcpy(idxDst, drawList->IdxBuffer.Data,
           drawList->IdxBuffer.Size * sizeof(ImDrawIdx));
    vtxDst += drawList->VtxBuffer.Size;
    idxDst += drawList->IdxBuffer.Size;
  }
  vertexTransferbuffer->unmap();
  indexTransferbuffer->unmap();

  gpu::BufferTransferInfo vertexBufferLocation = {};
  vertexBufferLocation.offset = 0;
  vertexBufferLocation.transferBuffer = vertexTransferbuffer;
  gpu::BufferTransferInfo indexBufferLocation = {};
  indexBufferLocation.offset = 0;
  indexBufferLocation.transferBuffer = indexTransferbuffer;

  gpu::BufferRegion vertexBufferRegion = {};
  vertexBufferRegion.buffer = fd->VertexBuffer;
  vertexBufferRegion.offset = 0;
  vertexBufferRegion.size = vertexSize;

  gpu::BufferRegion indexBufferRegion = {};
  indexBufferRegion.buffer = fd->IndexBuffer;
  indexBufferRegion.offset = 0;
  indexBufferRegion.size = indexSize;

  auto copyPass = command_buffer->beginCopyPass();
  copyPass->uploadBuffer(vertexBufferLocation, vertexBufferRegion, false);
  copyPass->uploadBuffer(indexBufferLocation, indexBufferRegion, false);
  command_buffer->endCopyPass(copyPass);
}
static void imGuiImplParanoixaSetupRenderState(
    ImDrawData *draw_data, Ptr<gpu::GraphicsPipeline> pipeline,
    Ptr<gpu::CommandBuffer> command_buffer, Ptr<gpu::RenderPass> render_pass,
    ImGuiImplParanoixaFrameData *fd, uint32_t fb_width, uint32_t fb_height) {
  ImGuiImplParanoixaData *bd = imGuiImplParanoixaGetBackendData();

  // Bind graphics pipeline
  render_pass->bindGraphicsPipeline(pipeline);

  // Bind Vertex And Index Buffers
  if (draw_data->TotalVtxCount > 0) {
    Array<gpu::BufferBinding> vertexBufferBindings(bd->InitInfo.Allocator);
    gpu::BufferBinding vertexBufferBinding;
    vertexBufferBinding.buffer = fd->VertexBuffer;
    vertexBufferBinding.offset = 0;
    vertexBufferBindings.push_back(vertexBufferBinding);

    Array<gpu::BufferBinding> indexBufferBindings(bd->InitInfo.Allocator);
    gpu::BufferBinding indexBufferBinding = {};
    indexBufferBinding.buffer = fd->IndexBuffer;
    indexBufferBinding.offset = 0;
    indexBufferBindings.push_back(indexBufferBinding);
    render_pass->bindVertexBuffers(0, vertexBufferBindings);
    render_pass->bindIndexBuffer(indexBufferBinding,
                                 sizeof(ImDrawIdx) == 2
                                     ? gpu::IndexElementSize::Uint16
                                     : gpu::IndexElementSize::Uint32);
  }

  // Setup viewport
  gpu::Viewport viewport = {};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = (float)fb_width;
  viewport.height = (float)fb_height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  render_pass->setViewport(viewport);

  // Setup scale and translation
  // Our visible imgui space lies from draw_data->DisplayPps (top left) to
  // draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is
  // (0,0) for single viewport apps.
  struct UBO {
    float scale[2];
    float translation[2];
  } ubo;
  ubo.scale[0] = 2.0f / draw_data->DisplaySize.x;
  ubo.scale[1] = 2.0f / draw_data->DisplaySize.y;
  ubo.translation[0] = -1.0f - draw_data->DisplayPos.x * ubo.scale[0];
  ubo.translation[1] = -1.0f - draw_data->DisplayPos.y * ubo.scale[1];
  command_buffer->pushVertexUniformData(0, &ubo, sizeof(UBO));
}

IMGUI_IMPL_API void imGuiImplParanoixaRenderDrawData(
    ImDrawData *draw_data, Ptr<gpu::CommandBuffer> command_buffer,
    Ptr<gpu::RenderPass> render_pass, Ptr<gpu::GraphicsPipeline> pipeline) {
  int fbWidth = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
  int fbHeight =
      (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
  if (fbWidth <= 0 || fbHeight <= 0)
    return;

  ImGuiImplParanoixaData *bd = imGuiImplParanoixaGetBackendData();
  ImGuiImplParanoixaFrameData *fd = &bd->MainWindowFrameData;

  if (pipeline == nullptr)
    pipeline = bd->Pipeline;

  imGuiImplParanoixaSetupRenderState(draw_data, pipeline, command_buffer,
                                     render_pass, fd, fbWidth, fbHeight);

  // Will project scissor/clipping rectangles into framebuffer space
  ImVec2 clipOff = draw_data->DisplayPos; // (0,0) unless using multi-viewports
  ImVec2 clipScale =
      draw_data->FramebufferScale; // (1,1) unless using retina display which
                                   // are often (2,2)

  // Render command lists
  // (Because we merged all buffers into a single one, we maintain our own
  // offset into them)
  int globalVtxOffset = 0;
  int globalIdxOffset = 0;
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const ImDrawList *drawList = draw_data->CmdLists[n];
    for (int cmdI = 0; cmdI < drawList->CmdBuffer.Size; cmdI++) {
      const ImDrawCmd *pcmd = &drawList->CmdBuffer[cmdI];
      if (pcmd->UserCallback != nullptr) {
        pcmd->UserCallback(drawList, pcmd);
      } else {
        // Project scissor/clipping rectangles into framebuffer space
        ImVec2 clipMin((pcmd->ClipRect.x - clipOff.x) * clipScale.x,
                       (pcmd->ClipRect.y - clipOff.y) * clipScale.y);
        ImVec2 clipMax((pcmd->ClipRect.z - clipOff.x) * clipScale.x,
                       (pcmd->ClipRect.w - clipOff.y) * clipScale.y);

        // Clamp to viewport as SDL_SetGPUScissor() won't accept values that are
        // off bounds
        if (clipMin.x < 0.0f) {
          clipMin.x = 0.0f;
        }
        if (clipMin.y < 0.0f) {
          clipMin.y = 0.0f;
        }
        if (clipMax.x > fbWidth) {
          clipMax.x = (float)fbWidth;
        }
        if (clipMax.y > fbHeight) {
          clipMax.y = (float)fbHeight;
        }
        if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
          continue;

        // Apply scissor/clipping rectangle
        render_pass->setScissor(clipMin.x, clipMin.y, clipMax.x - clipMin.x,
                                clipMax.y - clipMin.y);

        // Bind DescriptorSet with font or user texture
        Array<gpu::TextureSamplerBinding> bindings(bd->InitInfo.Allocator);
        auto *binding = (gpu::TextureSamplerBinding *)pcmd->GetTexID();
        bindings.push_back(*binding);
        render_pass->bindFragmentSamplers(0, bindings);

        // Draw
        render_pass->drawIndexedPrimitives(
            pcmd->ElemCount, 1, pcmd->IdxOffset + globalIdxOffset,
            pcmd->VtxOffset + globalVtxOffset, 0);
      }
    }
    globalIdxOffset += drawList->IdxBuffer.Size;
    globalVtxOffset += drawList->VtxBuffer.Size;
  }
  render_pass->setScissor(0, 0, fbWidth, fbHeight);
}

IMGUI_IMPL_API void imGuiImplParanoixaCreateDeviceObjects() {
  ImGuiImplParanoixaData *bd = imGuiImplParanoixaGetBackendData();
  ImGuiImplParanoixaInitInfo *v = &bd->InitInfo;

  if (!bd->FontSampler) {
    // Bilinear sampling is required by default. Set 'io.Fonts->Flags |=
    // ImFontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex =
    // false' to allow point/nearest sampling.
    gpu::Sampler::CreateInfo samplerInfo = {};
    samplerInfo.allocator = v->Allocator;
    samplerInfo.minFilter = gpu::Filter::Linear;
    samplerInfo.magFilter = gpu::Filter::Linear;
    samplerInfo.mipmapMode = gpu::MipmapMode::Linear;
    samplerInfo.addressModeU = gpu::AddressMode::ClampToEdge;
    samplerInfo.addressModeV = gpu::AddressMode::ClampToEdge;
    samplerInfo.addressModeW = gpu::AddressMode::ClampToEdge;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = -1000.0f;
    samplerInfo.maxLod = 1000.0f;
    samplerInfo.enableAnisotropy = false;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.enableCompare = false;

    bd->FontSampler = v->Device->createSampler(samplerInfo);
    bd->FontBinding.sampler = bd->FontSampler;
    IM_ASSERT(bd->FontSampler != nullptr &&
              "Failed to create font sampler, call SDL_GetError() for more "
              "information");
  }

  imGuiImplParanoixaCreateGraphicsPipeline();
  imGuiImplParanoixaCreateFontsTexture();
}

IMGUI_IMPL_API void imGuiImplParanoixaDestroyDeviceObjects() {}

IMGUI_IMPL_API void imGuiImplParanoixaCreateFontsTexture() {
  ImGuiIO &io = ImGui::GetIO();
  ImGuiImplParanoixaData *bd = imGuiImplParanoixaGetBackendData();
  ImGuiImplParanoixaInitInfo *v = &bd->InitInfo;

  // Destroy existing texture (if any)
  if (bd->FontTexture) {
    v->Device->waitForGpuIdle();
    imGuiImplParanoixaDestroyFontsTexture();
  }

  unsigned char *pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
  uint32_t uploadSize = width * height * 4 * sizeof(char);

  // Create the Image:
  {
    gpu::Texture::CreateInfo textureInfo = {};
    textureInfo.allocator = bd->InitInfo.Allocator;
    textureInfo.type = gpu::TextureType::Texture2D;
    textureInfo.format = gpu::TextureFormat::R8G8B8A8_UNORM;
    textureInfo.usage = gpu::TextureUsage::Sampler;
    textureInfo.width = width;
    textureInfo.height = height;
    textureInfo.layerCountOrDepth = 1;
    textureInfo.numLevels = 1;
    textureInfo.sampleCount = gpu::SampleCount::x1;

    bd->FontTexture = v->Device->createTexture(textureInfo);
    IM_ASSERT(bd->FontTexture && "Failed to create font texture");
  }

  // Assign the texture to the TextureSamplerBinding
  bd->FontBinding.texture = bd->FontTexture;

  // Create all the upload structures and upload:
  {
    gpu::TransferBuffer::CreateInfo transferbufferInfo = {};
    transferbufferInfo.allocator = v->Allocator;
    transferbufferInfo.usage = gpu::TransferBufferUsage::Upload;
    transferbufferInfo.size = uploadSize;

    Ptr<gpu::TransferBuffer> transferbuffer =
        v->Device->createTransferBuffer(transferbufferInfo);
    IM_ASSERT(transferbuffer != nullptr &&
              "Failed to create font transfer buffer");

    void *texturePtr = transferbuffer->map(false);
    memcpy(texturePtr, pixels, uploadSize);
    transferbuffer->unmap();

    gpu::TextureTransferInfo transferInfo = {};
    transferInfo.offset = 0;
    transferInfo.transferBuffer = transferbuffer;

    gpu::TextureRegion textureRegion = {};
    textureRegion.texture = bd->FontTexture;
    textureRegion.width = width;
    textureRegion.height = height;
    textureRegion.depth = 1;

    Ptr<gpu::CommandBuffer> cmd =
        v->Device->acquireCommandBuffer({bd->InitInfo.Allocator});
    Ptr<gpu::CopyPass> copyPass = cmd->beginCopyPass();
    copyPass->uploadTexture(transferInfo, textureRegion, false);
    cmd->endCopyPass(copyPass);
    v->Device->submitCommandBuffer(cmd);
  }

  // Store our identifier
  io.Fonts->SetTexID((ImTextureID)&bd->FontBinding);
}

IMGUI_IMPL_API void imGuiImplParanoixaDestroyFontsTexture() {
  return IMGUI_IMPL_API void();
}

} // namespace sinen
