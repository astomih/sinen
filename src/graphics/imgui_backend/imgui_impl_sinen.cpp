#include "imgui_impl_sinen.hpp"
#include "imgui_impl_sinen_shaders.hpp"

namespace sinen {
struct ImGuiImplParanoixaFrameData {
  rhi::Ptr<rhi::Buffer> VertexBuffer = nullptr;
  rhi::Ptr<rhi::Buffer> IndexBuffer = nullptr;
  uint32_t VertexBufferSize = 0;
  uint32_t IndexBufferSize = 0;
};

struct ImGuiImplParanoixaData {
  ImGuiImplParanoixaInitInfo InitInfo;

  // Graphics pipeline & shaders
  rhi::Ptr<rhi::Shader> VertexShader = nullptr;
  rhi::Ptr<rhi::Shader> FragmentShader = nullptr;
  rhi::Ptr<rhi::GraphicsPipeline> Pipeline = nullptr;

  // Font data
  rhi::Ptr<rhi::Sampler> FontSampler = nullptr;
  rhi::Ptr<rhi::Texture> FontTexture = nullptr;
  rhi::TextureSamplerBinding FontBinding = {nullptr, nullptr};

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

  rhi::Shader::CreateInfo vertex_shader_info = {};
  vertex_shader_info.allocator = v->Allocator;
  vertex_shader_info.entrypoint = "main";
  vertex_shader_info.stage = rhi::ShaderStage::Vertex;
  vertex_shader_info.numUniformBuffers = 1;
  vertex_shader_info.numStorageBuffers = 0;
  vertex_shader_info.numStorageTextures = 0;
  vertex_shader_info.numSamplers = 0;

  rhi::Shader::CreateInfo fragment_shader_info = {};
  fragment_shader_info.allocator = v->Allocator;
  fragment_shader_info.entrypoint = "main";
  fragment_shader_info.stage = rhi::ShaderStage::Fragment;
  fragment_shader_info.numSamplers = 1;
  fragment_shader_info.numStorageBuffers = 0;
  fragment_shader_info.numStorageTextures = 0;
  fragment_shader_info.numUniformBuffers = 0;

  if (driver == "vulkan") {
    vertex_shader_info.format = rhi::ShaderFormat::SPIRV;
    vertex_shader_info.data = spirv_vertex;
    vertex_shader_info.size = sizeof(spirv_vertex);
    fragment_shader_info.format = rhi::ShaderFormat::SPIRV;
    fragment_shader_info.data = spirv_fragment;
    fragment_shader_info.size = sizeof(spirv_fragment);
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
  bd->VertexShader = v->Device->createShader(vertex_shader_info);
  bd->FragmentShader = v->Device->createShader(fragment_shader_info);
  IM_ASSERT(bd->VertexShader != nullptr);
  IM_ASSERT(bd->FragmentShader != nullptr);
}
static void imGuiImplParanoixaCreateGraphicsPipeline() {
  ImGuiImplParanoixaData *bd = imGuiImplParanoixaGetBackendData();
  ImGuiImplParanoixaInitInfo *v = &bd->InitInfo;
  imguiImplParanoixaCreateShaders();

  rhi::Array<rhi::VertexBufferDescription> vertex_buffer_desc(v->Allocator);
  vertex_buffer_desc.resize(1);
  vertex_buffer_desc[0].slot = 0;
  vertex_buffer_desc[0].inputRate = rhi::VertexInputRate::Vertex;
  vertex_buffer_desc[0].instanceStepRate = 0;
  vertex_buffer_desc[0].pitch = sizeof(ImDrawVert);

  rhi::Array<rhi::VertexAttribute> vertex_attributes(v->Allocator);
  vertex_attributes.resize(3);
  vertex_attributes[0].bufferSlot = 0;
  vertex_attributes[0].format = rhi::VertexElementFormat::Float2;
  vertex_attributes[0].location = 0;
  vertex_attributes[0].offset = offsetof(ImDrawVert, pos);

  vertex_attributes[1].bufferSlot = 0;
  vertex_attributes[1].format = rhi::VertexElementFormat::Float2;
  vertex_attributes[1].location = 1;
  vertex_attributes[1].offset = offsetof(ImDrawVert, uv);

  vertex_attributes[2].bufferSlot = 0;
  vertex_attributes[2].format = rhi::VertexElementFormat::UByte4_NORM;
  vertex_attributes[2].location = 2;
  vertex_attributes[2].offset = offsetof(ImDrawVert, col);

  rhi::VertexInputState vertex_input_state{v->Allocator};
  vertex_input_state.vertexAttributes = vertex_attributes;
  vertex_input_state.vertexBufferDescriptions = vertex_buffer_desc;

  rhi::RasterizerState rasterizer_state = {};
  rasterizer_state.fillMode = rhi::FillMode::Fill;
  rasterizer_state.cullMode = rhi::CullMode::None;
  rasterizer_state.frontFace = rhi::FrontFace::CounterClockwise;
  rasterizer_state.enableDepthBias = false;
  rasterizer_state.enableDepthClip = false;

  rhi::MultiSampleState multisample_state{};
  multisample_state.sampleCount = v->MSAASamples;
  multisample_state.enableMask = false;

  rhi::DepthStencilState depth_stencil_state = {};
  depth_stencil_state.enableDepthTest = false;
  depth_stencil_state.enableDepthWrite = false;
  depth_stencil_state.enableStencilTest = false;

  rhi::ColorTargetBlendState blend_state = {};
  blend_state.enableBlend = true;
  blend_state.srcColorBlendFactor = rhi::BlendFactor::SrcAlpha;
  blend_state.dstColorBlendFactor = rhi::BlendFactor::OneMinusSrcAlpha;
  blend_state.colorBlendOp = rhi::BlendOp::Add;
  blend_state.srcAlphaBlendFactor = rhi::BlendFactor::One;
  blend_state.dstAlphaBlendFactor = rhi::BlendFactor::OneMinusSrcAlpha;
  blend_state.alphaBlendOp = rhi::BlendOp::Add;
  blend_state.colorWriteMask = rhi::ColorComponent::R | rhi::ColorComponent::G |
                               rhi::ColorComponent::B | rhi::ColorComponent::A;

  rhi::Array<rhi::ColorTargetDescription> color_target_desc(v->Allocator);
  color_target_desc.resize(1);
  color_target_desc[0].format = v->ColorTargetFormat;
  color_target_desc[0].blendState = blend_state;

  rhi::TargetInfo target_info = {v->Allocator};
  target_info.colorTargetDescriptions = color_target_desc;
  target_info.hasDepthStencilTarget = false;

  rhi::GraphicsPipeline::CreateInfo pipeline_info = {v->Allocator};
  pipeline_info.allocator = v->Allocator;
  pipeline_info.vertexShader = bd->VertexShader;
  pipeline_info.fragmentShader = bd->FragmentShader;
  pipeline_info.vertexInputState = vertex_input_state;
  pipeline_info.primitiveType = rhi::PrimitiveType::TriangleList;
  pipeline_info.rasterizerState = rasterizer_state;
  pipeline_info.multiSampleState = multisample_state;
  pipeline_info.depthStencilState = depth_stencil_state;
  pipeline_info.targetInfo = target_info;

  bd->Pipeline = v->Device->createGraphicsPipeline(pipeline_info);
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
  IM_ASSERT(info->ColorTargetFormat != rhi::TextureFormat::Invalid);

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
static void createOrResizeBuffer(rhi::Ptr<rhi::Buffer> &buffer,
                                 uint32_t *old_size, uint32_t new_size,
                                 rhi::BufferUsage usage) {
  ImGuiImplParanoixaData *bd = imGuiImplParanoixaGetBackendData();
  ImGuiImplParanoixaInitInfo *v = &bd->InitInfo;

  // Even though this is fairly rarely called.
  v->Device->waitForGpuIdle();

  rhi::Buffer::CreateInfo buffer_info = {};
  buffer_info.allocator = v->Allocator;
  buffer_info.usage = usage;
  buffer_info.size = new_size;
  // buffer_info.props = 0;
  buffer = v->Device->createBuffer(buffer_info);
  *old_size = new_size;
  IM_ASSERT(buffer != nullptr && "Failed to create GPU Buffer");
}

IMGUI_IMPL_API void
imGuiImplParanoixaPrepareDrawData(ImDrawData *draw_data,
                                  rhi::Ptr<rhi::CommandBuffer> command_buffer) {
  // Avoid rendering when minimized, scale coordinates for retina displays
  // (screen coordinates != framebuffer coordinates)
  int fb_width =
      (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
  int fb_height =
      (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
  if (fb_width <= 0 || fb_height <= 0 || draw_data->TotalVtxCount <= 0)
    return;

  ImGuiImplParanoixaData *bd = imGuiImplParanoixaGetBackendData();
  ImGuiImplParanoixaInitInfo *v = &bd->InitInfo;
  ImGuiImplParanoixaFrameData *fd = &bd->MainWindowFrameData;

  uint32_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
  uint32_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
  if (fd->VertexBuffer == nullptr || fd->VertexBufferSize < vertex_size)
    createOrResizeBuffer(fd->VertexBuffer, &fd->VertexBufferSize, vertex_size,
                         rhi::BufferUsage::Vertex);
  IM_ASSERT(fd->VertexBuffer != nullptr &&
            "Failed to create the vertex buffer");
  if (fd->IndexBuffer == nullptr || fd->IndexBufferSize < index_size)
    createOrResizeBuffer(fd->IndexBuffer, &fd->IndexBufferSize, index_size,
                         rhi::BufferUsage::Index);
  IM_ASSERT(fd->IndexBuffer != nullptr && "Failed to create the index buffer");

  // FIXME: It feels like more code could be shared there.
  rhi::TransferBuffer::CreateInfo vertex_transferbuffer_info = {};
  vertex_transferbuffer_info.allocator = v->Allocator;
  vertex_transferbuffer_info.usage = rhi::TransferBufferUsage::Upload;
  vertex_transferbuffer_info.size = vertex_size;
  rhi::TransferBuffer::CreateInfo index_transferbuffer_info = {};
  index_transferbuffer_info.allocator = v->Allocator;
  index_transferbuffer_info.usage = rhi::TransferBufferUsage::Upload;
  index_transferbuffer_info.size = index_size;

  auto vertex_transferbuffer =
      v->Device->createTransferBuffer(vertex_transferbuffer_info);
  IM_ASSERT(vertex_transferbuffer != nullptr &&
            "Failed to create the vertex transfer buffer");
  auto index_transferbuffer =
      v->Device->createTransferBuffer(index_transferbuffer_info);
  IM_ASSERT(index_transferbuffer != nullptr &&
            "Failed to create the index transfer buffer");

  ImDrawVert *vtx_dst = (ImDrawVert *)vertex_transferbuffer->map(true);
  ImDrawIdx *idx_dst = (ImDrawIdx *)index_transferbuffer->map(true);
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const ImDrawList *draw_list = draw_data->CmdLists[n];
    memcpy(vtx_dst, draw_list->VtxBuffer.Data,
           draw_list->VtxBuffer.Size * sizeof(ImDrawVert));
    memcpy(idx_dst, draw_list->IdxBuffer.Data,
           draw_list->IdxBuffer.Size * sizeof(ImDrawIdx));
    vtx_dst += draw_list->VtxBuffer.Size;
    idx_dst += draw_list->IdxBuffer.Size;
  }
  vertex_transferbuffer->unmap();
  index_transferbuffer->unmap();

  rhi::BufferTransferInfo vertex_buffer_location = {};
  vertex_buffer_location.offset = 0;
  vertex_buffer_location.transferBuffer = vertex_transferbuffer;
  rhi::BufferTransferInfo index_buffer_location = {};
  index_buffer_location.offset = 0;
  index_buffer_location.transferBuffer = index_transferbuffer;

  rhi::BufferRegion vertex_buffer_region = {};
  vertex_buffer_region.buffer = fd->VertexBuffer;
  vertex_buffer_region.offset = 0;
  vertex_buffer_region.size = vertex_size;

  rhi::BufferRegion index_buffer_region = {};
  index_buffer_region.buffer = fd->IndexBuffer;
  index_buffer_region.offset = 0;
  index_buffer_region.size = index_size;

  auto copy_pass = command_buffer->beginCopyPass();
  copy_pass->uploadBuffer(vertex_buffer_location, vertex_buffer_region, false);
  copy_pass->uploadBuffer(index_buffer_location, index_buffer_region, false);
  command_buffer->endCopyPass(copy_pass);
}
static void imGuiImplParanoixaSetupRenderState(
    ImDrawData *draw_data, rhi::Ptr<rhi::GraphicsPipeline> pipeline,
    rhi::Ptr<rhi::CommandBuffer> command_buffer,
    rhi::Ptr<rhi::RenderPass> render_pass, ImGuiImplParanoixaFrameData *fd,
    uint32_t fb_width, uint32_t fb_height) {
  ImGuiImplParanoixaData *bd = imGuiImplParanoixaGetBackendData();

  // Bind graphics pipeline
  render_pass->bindGraphicsPipeline(pipeline);

  // Bind Vertex And Index Buffers
  if (draw_data->TotalVtxCount > 0) {
    rhi::Array<rhi::BufferBinding> vertex_buffer_bindings(
        bd->InitInfo.Allocator);
    rhi::BufferBinding vertex_buffer_binding;
    vertex_buffer_binding.buffer = fd->VertexBuffer;
    vertex_buffer_binding.offset = 0;
    vertex_buffer_bindings.push_back(vertex_buffer_binding);

    rhi::Array<rhi::BufferBinding> index_buffer_bindings(
        bd->InitInfo.Allocator);
    rhi::BufferBinding index_buffer_binding = {};
    index_buffer_binding.buffer = fd->IndexBuffer;
    index_buffer_binding.offset = 0;
    index_buffer_bindings.push_back(index_buffer_binding);
    render_pass->bindVertexBuffers(0, vertex_buffer_bindings);
    render_pass->bindIndexBuffer(index_buffer_binding,
                                 sizeof(ImDrawIdx) == 2
                                     ? rhi::IndexElementSize::Uint16
                                     : rhi::IndexElementSize::Uint32);
  }

  // Setup viewport
  rhi::Viewport viewport = {};
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
  command_buffer->pushUniformData(0, &ubo, sizeof(UBO));
}

IMGUI_IMPL_API void
imGuiImplParanoixaRenderDrawData(ImDrawData *draw_data,
                                 rhi::Ptr<rhi::CommandBuffer> command_buffer,
                                 rhi::Ptr<rhi::RenderPass> render_pass,
                                 rhi::Ptr<rhi::GraphicsPipeline> pipeline) {
  int fb_width =
      (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
  int fb_height =
      (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
  if (fb_width <= 0 || fb_height <= 0)
    return;

  ImGuiImplParanoixaData *bd = imGuiImplParanoixaGetBackendData();
  ImGuiImplParanoixaFrameData *fd = &bd->MainWindowFrameData;

  if (pipeline == nullptr)
    pipeline = bd->Pipeline;

  imGuiImplParanoixaSetupRenderState(draw_data, pipeline, command_buffer,
                                     render_pass, fd, fb_width, fb_height);

  // Will project scissor/clipping rectangles into framebuffer space
  ImVec2 clip_off = draw_data->DisplayPos; // (0,0) unless using multi-viewports
  ImVec2 clip_scale =
      draw_data->FramebufferScale; // (1,1) unless using retina display which
                                   // are often (2,2)

  // Render command lists
  // (Because we merged all buffers into a single one, we maintain our own
  // offset into them)
  int global_vtx_offset = 0;
  int global_idx_offset = 0;
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const ImDrawList *draw_list = draw_data->CmdLists[n];
    for (int cmd_i = 0; cmd_i < draw_list->CmdBuffer.Size; cmd_i++) {
      const ImDrawCmd *pcmd = &draw_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback != nullptr) {
        pcmd->UserCallback(draw_list, pcmd);
      } else {
        // Project scissor/clipping rectangles into framebuffer space
        ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
                        (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
        ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
                        (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

        // Clamp to viewport as SDL_SetGPUScissor() won't accept values that are
        // off bounds
        if (clip_min.x < 0.0f) {
          clip_min.x = 0.0f;
        }
        if (clip_min.y < 0.0f) {
          clip_min.y = 0.0f;
        }
        if (clip_max.x > fb_width) {
          clip_max.x = (float)fb_width;
        }
        if (clip_max.y > fb_height) {
          clip_max.y = (float)fb_height;
        }
        if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
          continue;

        // Apply scissor/clipping rectangle
        render_pass->setScissor(clip_min.x, clip_min.y, clip_max.x - clip_min.x,
                                clip_max.y - clip_min.y);

        // Bind DescriptorSet with font or user texture
        rhi::Array<rhi::TextureSamplerBinding> bindings(bd->InitInfo.Allocator);
        auto *binding = (rhi::TextureSamplerBinding *)pcmd->GetTexID();
        bindings.push_back(*binding);
        render_pass->bindFragmentSamplers(0, bindings);

        // Draw
        render_pass->drawIndexedPrimitives(
            pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset,
            pcmd->VtxOffset + global_vtx_offset, 0);
      }
    }
    global_idx_offset += draw_list->IdxBuffer.Size;
    global_vtx_offset += draw_list->VtxBuffer.Size;
  }
  render_pass->setScissor(0, 0, fb_width, fb_height);
}

IMGUI_IMPL_API void imGuiImplParanoixaCreateDeviceObjects() {
  ImGuiImplParanoixaData *bd = imGuiImplParanoixaGetBackendData();
  ImGuiImplParanoixaInitInfo *v = &bd->InitInfo;

  if (!bd->FontSampler) {
    // Bilinear sampling is required by default. Set 'io.Fonts->Flags |=
    // ImFontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex =
    // false' to allow point/nearest sampling.
    rhi::Sampler::CreateInfo sampler_info = {};
    sampler_info.allocator = v->Allocator;
    sampler_info.minFilter = rhi::Filter::Linear;
    sampler_info.magFilter = rhi::Filter::Linear;
    sampler_info.mipmapMode = rhi::MipmapMode::Linear;
    sampler_info.addressModeU = rhi::AddressMode::ClampToEdge;
    sampler_info.addressModeV = rhi::AddressMode::ClampToEdge;
    sampler_info.addressModeW = rhi::AddressMode::ClampToEdge;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = -1000.0f;
    sampler_info.maxLod = 1000.0f;
    sampler_info.enableAnisotropy = false;
    sampler_info.maxAnisotropy = 1.0f;
    sampler_info.enableCompare = false;

    bd->FontSampler = v->Device->createSampler(sampler_info);
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
  uint32_t upload_size = width * height * 4 * sizeof(char);

  // Create the Image:
  {
    rhi::Texture::CreateInfo texture_info = {};
    texture_info.allocator = bd->InitInfo.Allocator;
    texture_info.type = rhi::TextureType::Texture2D;
    texture_info.format = rhi::TextureFormat::R8G8B8A8_UNORM;
    texture_info.usage = rhi::TextureUsage::Sampler;
    texture_info.width = width;
    texture_info.height = height;
    texture_info.layerCountOrDepth = 1;
    texture_info.numLevels = 1;
    texture_info.sampleCount = rhi::SampleCount::x1;

    bd->FontTexture = v->Device->createTexture(texture_info);
    IM_ASSERT(bd->FontTexture && "Failed to create font texture");
  }

  // Assign the texture to the TextureSamplerBinding
  bd->FontBinding.texture = bd->FontTexture;

  // Create all the upload structures and upload:
  {
    rhi::TransferBuffer::CreateInfo transferbuffer_info = {};
    transferbuffer_info.allocator = v->Allocator;
    transferbuffer_info.usage = rhi::TransferBufferUsage::Upload;
    transferbuffer_info.size = upload_size;

    rhi::Ptr<rhi::TransferBuffer> transferbuffer =
        v->Device->createTransferBuffer(transferbuffer_info);
    IM_ASSERT(transferbuffer != nullptr &&
              "Failed to create font transfer buffer");

    void *texture_ptr = transferbuffer->map(false);
    memcpy(texture_ptr, pixels, upload_size);
    transferbuffer->unmap();

    rhi::TextureTransferInfo transfer_info = {};
    transfer_info.offset = 0;
    transfer_info.transferBuffer = transferbuffer;

    rhi::TextureRegion texture_region = {};
    texture_region.texture = bd->FontTexture;
    texture_region.width = width;
    texture_region.height = height;
    texture_region.depth = 1;

    rhi::Ptr<rhi::CommandBuffer> cmd =
        v->Device->acquireCommandBuffer({bd->InitInfo.Allocator});
    rhi::Ptr<rhi::CopyPass> copy_pass = cmd->beginCopyPass();
    copy_pass->uploadTexture(transfer_info, texture_region, false);
    cmd->endCopyPass(copy_pass);
    v->Device->submitCommandBuffer(cmd);
  }

  // Store our identifier
  io.Fonts->SetTexID((ImTextureID)&bd->FontBinding);
}

IMGUI_IMPL_API void imGuiImplParanoixaDestroyFontsTexture() {
  return IMGUI_IMPL_API void();
}

} // namespace sinen
