#include "graphics_system.hpp"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>

#include <asset/model/vertex.hpp>
#include <asset/model/vertex_array.hpp>
#include <asset/texture/render_texture.hpp>
#include <cassert>
#include <core/io/asset_io.hpp>
#include <core/io/asset_type.hpp>
#include <core/logger/logger.hpp>
#include <cstdint>
#include <graphics/drawable/instance_data.hpp>
#include <graphics/graphics.hpp>
#include <imgui_impl_paranoixa.hpp>
#include <memory>
#include <platform/platform.hpp>
#include <platform/window/window.hpp>

#include "../asset/font/default/mplus-1p-medium.ttf.hpp"
#include "../asset/model/default_model_creator.hpp"
#include "../asset/model/model_data.hpp"
#include "../asset/script/script_system.hpp"
#include "../asset/texture/texture_data.hpp"
#include "../main_system.hpp"
#include "../platform/window/window_system.hpp"
#include "graphics/graphics.hpp"
#include "imgui_internal.h"
#include "paranoixa/paranoixa.hpp"

namespace sinen {
PxDrawable::PxDrawable(px::Allocator *allocator)
    : allocator(allocator), vertexBuffers(allocator), indexBuffer(),
      textureSamplers(allocator) {}
Color GraphicsSystem::clearColor = Palette::black();
// Renderer
bool GraphicsSystem::showImGui = false;
std::list<std::function<void()>> GraphicsSystem::m_imgui_function;
Model GraphicsSystem::box = Model();
Model GraphicsSystem::sprite = Model();
void GraphicsSystem::unload_data() {}
void GraphicsSystem::initialize() {
  backend = px::Paranoixa::CreateBackend(allocator, px::GraphicsAPI::SDLGPU);
  px::Device::CreateInfo info{};
  info.allocator = allocator;
  info.debugMode = true;
  device = backend->CreateDevice(info);
  auto *window = WindowSystem::get_sdl_window();
  device->ClaimWindow(window);
  IMGUI_CHECKVERSION();
  auto *context = ImGui::CreateContext();
  context->IO.Fonts->AddFontFromMemoryTTF(
      (void *)mplus_1p_medium_ttf, mplus_1p_medium_ttf_len, 32.0f, nullptr,
      context->IO.Fonts->GetGlyphRangesJapanese());
  context->IO.IniFilename = nullptr;
  auto *ime_data = &context->PlatformImeData;
  ime_data->WantVisible = true;
  ImGui_ImplSDL3_InitForSDLGPU(window);
  ImGui_ImplParanoixa_InitInfo init_info = {};
  init_info.Allocator = allocator;
  init_info.Device = device;
#ifdef _WIN32
  init_info.ColorTargetFormat = px::TextureFormat::B8G8R8A8_UNORM;
#else
  init_info.ColorTargetFormat = px::TextureFormat::R8G8B8A8_UNORM;
#endif
  init_info.MSAASamples = px::SampleCount::x1;
  GraphicsSystem::prepare_imgui();
  ImGui_ImplParanoixa_Init(&init_info);

  Shader vs;
  vs.loadDefaultVertexShader();
  Shader vsInstanced;
  vsInstanced.loadDefaultVertexInstanceShader();
  Shader fs;
  fs.loadDefaultFragmentShader();

  pipeline3D.setVertexShader(vs);
  pipeline3D.setVertexInstancedShader(vsInstanced);
  pipeline3D.setFragmentShader(fs);
  pipeline3D.build();
  currentPipeline3D = pipeline3D;

  pipeline2D.setVertexShader(vs);
  pipeline2D.setFragmentShader(fs);
  pipeline2D.build();
  currentPipeline2D = pipeline2D;

  // Create depth stencil target
  {
    px::Texture::CreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.allocator = allocator;
    depthStencilCreateInfo.width = static_cast<uint32_t>(Window::size().x);
    depthStencilCreateInfo.height = static_cast<uint32_t>(Window::size().y);
    depthStencilCreateInfo.layerCountOrDepth = 1;
    depthStencilCreateInfo.type = px::TextureType::Texture2D;
    depthStencilCreateInfo.usage = px::TextureUsage::DepthStencilTarget;
    depthStencilCreateInfo.format = px::TextureFormat::D32_FLOAT_S8_UINT;
    depthStencilCreateInfo.numLevels = 1;
    depthStencilCreateInfo.sampleCount = px::SampleCount::x1;
    depthTexture = device->CreateTexture(depthStencilCreateInfo);
  }

  // Default sampler
  px::Sampler::CreateInfo samplerInfo{};
  samplerInfo.allocator = allocator;
  samplerInfo.minFilter = px::Filter::Nearest;
  samplerInfo.magFilter = px::Filter::Nearest;
  samplerInfo.addressModeU = px::AddressMode::Repeat;
  samplerInfo.addressModeV = px::AddressMode::Repeat;
  samplerInfo.maxAnisotropy = 1.f;
  sampler = device->CreateSampler(samplerInfo);

  colorTargets.push_back({});
  depthStencilInfo.texture = depthTexture;
  depthStencilInfo.loadOp = px::LoadOp::Clear;
  depthStencilInfo.storeOp = px::StoreOp::Store;
  depthStencilInfo.clearDepth = 1.0f;
  depthStencilInfo.clearStencil = 0;
  depthStencilInfo.cycle = false;
  depthStencilInfo.stencilLoadOp = px::LoadOp::Clear;
  depthStencilInfo.stencilStoreOp = px::StoreOp::Store;
  setup_shapes();
}

void GraphicsSystem::shutdown() {
  vertexArrays.clear();
  device.reset();
  backend.reset();
}

void GraphicsSystem::render() {
  auto commandBuffer = device->AcquireCommandBuffer({allocator});
  if (commandBuffer == nullptr) {
    return;
  }
  auto swapchainTexture = device->AcquireSwapchainTexture(commandBuffer);
  if (swapchainTexture == nullptr) {
    return;
  }
  mainCommandBuffer = commandBuffer;
  currentCommandBuffer = commandBuffer;
  colorTargets[0].texture = swapchainTexture;
  currentColorTargets = colorTargets;
  if (WindowSystem::resized()) {
    px::Texture::CreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.allocator = allocator;
    depthStencilCreateInfo.width = Window::size().x;
    depthStencilCreateInfo.height = Window::size().y;
    depthStencilCreateInfo.layerCountOrDepth = 1;
    depthStencilCreateInfo.type = px::TextureType::Texture2D;
    depthStencilCreateInfo.usage = px::TextureUsage::DepthStencilTarget;
    depthStencilCreateInfo.format = px::TextureFormat::D32_FLOAT_S8_UINT;
    depthStencilCreateInfo.numLevels = 1;
    depthStencilCreateInfo.sampleCount = px::SampleCount::x1;
    depthTexture = device->CreateTexture(depthStencilCreateInfo);
    depthStencilInfo.texture = depthTexture;
  }
  currentDepthStencilInfo = depthStencilInfo;

  ImGui_ImplParanoixa_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
  for (auto &func : GraphicsSystem::get_imgui_function()) {
    func();
  }
  // Rendering
  ImGui::Render();
  ImDrawData *draw_data = ImGui::GetDrawData();

  Imgui_ImplParanoixa_PrepareDrawData(draw_data, commandBuffer);
  isFrameStarted = true;
  objectCount = 0;
  ScriptSystem::DrawScene();
  if (objectCount > 0 && !isDraw2D) {
    commandBuffer->EndRenderPass(currentRenderPass);
  }

  {
    if (isFrameStarted || (objectCount > 0 && !isDraw2D)) {
      colorTargets[0].loadOp = px::LoadOp::Load;
      currentRenderPass = commandBuffer->BeginRenderPass(colorTargets, {});
    }
    auto renderPass = currentRenderPass;
    renderPass->SetViewport(
        px::Viewport{0, 0, Window::size().x, Window::size().y, 0, 1});
    renderPass->SetScissor(0, 0, Window::size().x, Window::size().y);
    // Render ImGui
    ImGui_ImplParanoixa_RenderDrawData(draw_data, commandBuffer, renderPass);
    commandBuffer->EndRenderPass(renderPass);
  }
  device->SubmitCommandBuffer(commandBuffer);
  device->WaitForGPUIdle();
}
void GraphicsSystem::Draw2D(const sinen::Draw2D &draw2D) {
  auto ratio = camera2D.windowRatio();
  {
    auto t = glm::translate(glm::mat4(1.0f),
                            glm::vec3(draw2D.position.x * ratio.x,
                                      draw2D.position.y * ratio.y, 0.0f));
    auto quaternion = glm::angleAxis(glm::radians(draw2D.rotation),
                                     glm::vec3(0.0f, 0.0f, -1.0f));
    auto r = glm::toMat4(quaternion);

    auto s =
        glm::scale(glm::mat4(1.0f), glm::vec3(draw2D.scale.x * 0.5f,
                                              draw2D.scale.y * 0.5f, 1.0f));

    draw2D.obj->param.world = glm::transpose(t * r * s);
  }
  draw2D.obj->material = draw2D.material;
  auto viewproj = glm::mat4(1.0f);

  auto screen_size = camera2D.size();
  viewproj[0][0] = 2.f / Window::size().x;
  viewproj[1][1] = 2.f / Window::size().y;
  draw2D.obj->param.proj = glm::transpose(viewproj);
  draw2D.obj->param.view = glm::mat4(1.f);
  if (GetModelData(draw2D.model.data)->vertexBuffer == nullptr) {
    draw2D.obj->model = GraphicsSystem::sprite;
  } else
    draw2D.obj->model = draw2D.model;
  for (auto &i : draw2D.worlds) {
    auto t = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(i.position.x * ratio.x, i.position.y * ratio.y, 0.0f));
    auto quaternion =
        glm::angleAxis(glm::radians(i.rotation), glm::vec3(0.0f, 0.0f, -1.0f));
    auto r = glm::toMat4(quaternion);
    auto s = glm::scale(glm::mat4(1.0f),
                        glm::vec3(i.scale.x * 0.5f, i.scale.y * 0.5f, 1.0f));

    auto world = t * r * s;

    InstanceData insdata{};
    draw2D.obj->worldToInstanceData(glm::transpose(world), insdata);
    draw2D.obj->data.push_back(insdata);
  }
  objectCount++;
  if (isFrameStarted || currentRenderPass == nullptr) {
    colorTargets[0].loadOp = px::LoadOp::Clear;
    currentColorTargets = colorTargets;
    currentRenderPass =
        currentCommandBuffer->BeginRenderPass(currentColorTargets, {});
    auto renderPass = currentRenderPass;
    renderPass->SetViewport(
        px::Viewport{0, 0, Window::size().x, Window::size().y, 0, 1});
    renderPass->SetScissor(0, 0, static_cast<int32_t>(Window::size().x),
                           static_cast<int32_t>(Window::size().y));
    isFrameStarted = false;
    isDraw2D = true;
    isDefaultPipeline = true;
  } else if (isDefaultPipeline && !isDraw2D) {
    currentCommandBuffer->EndRenderPass(currentRenderPass);
    colorTargets[0].loadOp = px::LoadOp::Load;
    currentColorTargets = colorTargets;
    currentRenderPass =
        currentCommandBuffer->BeginRenderPass(currentColorTargets, {});
    auto renderPass = currentRenderPass;
    renderPass->SetViewport(
        px::Viewport{0, 0, Window::size().x, Window::size().y, 0, 1});
    renderPass->SetScissor(0, 0, static_cast<int32_t>(Window::size().x),
                           static_cast<int32_t>(Window::size().y));
    isDraw2D = true;
    isDefaultPipeline = true;
  }

  PxDrawable drawable{allocator};
  drawable.drawable = draw2D.obj;

  for (const auto &texture : draw2D.obj->material.getTextures()) {
    auto nativeTexture = std::static_pointer_cast<px::Texture>(
        GetTexData(texture.textureData)->texture);
    drawable.textureSamplers.push_back(px::TextureSamplerBinding{
        .sampler = sampler, .texture = nativeTexture});
  }

  auto modelData = GetModelData(draw2D.obj->model.data);
  assert(modelData->vertexBuffer != nullptr);
  assert(modelData->indexBuffer != nullptr);

  drawable.vertexBuffers.emplace_back(
      px::BufferBinding{.buffer = modelData->vertexBuffer, .offset = 0});
  drawable.indexBuffer =
      px::BufferBinding{.buffer = modelData->indexBuffer, .offset = 0};

  auto commandBuffer = currentCommandBuffer;
  auto renderPass = currentRenderPass;
  renderPass->BindGraphicsPipeline(currentPipeline2D.get());
  renderPass->BindFragmentSamplers(0, drawable.textureSamplers);
  renderPass->BindVertexBuffers(0, drawable.vertexBuffers);
  renderPass->BindIndexBuffer(drawable.indexBuffer,
                              px::IndexElementSize::Uint32);

  auto param = drawable.drawable->param;
  commandBuffer->PushVertexUniformData(0, &param, sizeof(Drawable::parameter));
  renderPass->DrawIndexedPrimitives(
      GetModelData(drawable.drawable->model.data)->v_array.indexCount, 1, 0, 0,
      0);
}

void GraphicsSystem::Draw3D(const sinen::Draw3D &draw3D) {
  draw3D.obj->material = draw3D.material;
  {
    const auto t = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(draw3D.position.x, draw3D.position.y, draw3D.position.z));
    const auto rotationX = glm::angleAxis(glm::radians(draw3D.rotation.x),
                                          glm::vec3(1.0f, 0.0f, 0.0f));
    const auto rotationY = glm::angleAxis(glm::radians(draw3D.rotation.y),
                                          glm::vec3(0.0f, 1.0f, 0.0f));
    const auto rotationZ = glm::angleAxis(glm::radians(draw3D.rotation.z),
                                          glm::vec3(0.0f, 0.0f, 1.0f));
    const auto r = glm::toMat4(rotationX * rotationY * rotationZ);

    const auto s =
        glm::scale(glm::mat4(1.0f),
                   glm::vec3(draw3D.scale.x, draw3D.scale.y, draw3D.scale.z));

    auto world = t * r * s;
    draw3D.obj->param.world = glm::transpose(world);
    draw3D.obj->param.proj = glm::transpose(camera.getProjection());
    draw3D.obj->param.view = glm::transpose(camera.getView());
  }
  if (GetModelData(draw3D.model.data)->vertexBuffer == nullptr) {
    draw3D.obj->model = GraphicsSystem::box;
  } else
    draw3D.obj->model = draw3D.model;
  for (auto &i : draw3D.worlds) {
    InstanceData insdata{};
    auto t = glm::translate(
        glm::mat4(1.0f), glm::vec3(i.position.x, i.position.y, i.position.z));
    auto rotationX =
        glm::angleAxis(glm::radians(i.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    auto rotationY =
        glm::angleAxis(glm::radians(i.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    auto rotationZ =
        glm::angleAxis(glm::radians(i.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    auto r = glm::toMat4(rotationX * rotationY * rotationZ);

    auto s =
        glm::scale(glm::mat4(1.0f), glm::vec3(i.scale.x, i.scale.y, i.scale.z));

    auto world = t * r * s;

    draw3D.obj->worldToInstanceData(glm::transpose(world), insdata);
    draw3D.obj->data.push_back(insdata);
  }
  objectCount++;
  if (isDefaultPipeline && (isFrameStarted || currentRenderPass == nullptr)) {
    colorTargets[0].loadOp = px::LoadOp::Clear;
    currentColorTargets = colorTargets;
    currentDepthStencilInfo = depthStencilInfo;
    currentRenderPass = currentCommandBuffer->BeginRenderPass(
        currentColorTargets, currentDepthStencilInfo);
    auto renderPass = currentRenderPass;
    renderPass->SetViewport(
        px::Viewport{0, 0, Window::size().x, Window::size().y, 0, 1});
    renderPass->SetScissor(0, 0, Window::size().x, Window::size().y);
    isFrameStarted = false;
    isDraw2D = false;
    isDefaultPipeline = true;
  } else if (isDefaultPipeline && isDraw2D) {
    currentCommandBuffer->EndRenderPass(currentRenderPass);
    colorTargets[0].loadOp = px::LoadOp::Load;
    currentColorTargets = colorTargets;
    currentDepthStencilInfo = depthStencilInfo;
    currentRenderPass = currentCommandBuffer->BeginRenderPass(
        currentColorTargets, currentDepthStencilInfo);
    auto renderPass = currentRenderPass;
    renderPass->SetViewport(
        px::Viewport{0, 0, Window::size().x, Window::size().y, 0, 1});
    renderPass->SetScissor(0, 0, Window::size().x, Window::size().y);
    isDraw2D = false;
    isDefaultPipeline = true;
  }
  PxDrawable drawable{allocator};
  drawable.drawable = draw3D.obj;
  for (const auto &texture : draw3D.obj->material.getTextures()) {
    auto nativeTexture = std::static_pointer_cast<px::Texture>(
        GetTexData(texture.textureData)->texture);
    drawable.textureSamplers.push_back(px::TextureSamplerBinding{
        .sampler = sampler, .texture = nativeTexture});
  }

  bool isInstance = drawable.drawable->size() > 0;
  if (isInstance) {
    px::Buffer::CreateInfo instanceBufferInfo{};
    instanceBufferInfo.allocator = allocator;
    instanceBufferInfo.size = drawable.drawable->size();
    instanceBufferInfo.usage = px::BufferUsage::Vertex;
    auto instanceBuffer = device->CreateBuffer(instanceBufferInfo);
    Ptr<px::TransferBuffer> transferBuffer;
    {
      px::TransferBuffer::CreateInfo info{};
      info.allocator = allocator;
      info.size = drawable.drawable->size();
      info.usage = px::TransferBufferUsage::Upload;
      transferBuffer = device->CreateTransferBuffer(info);
      auto *pMapped = transferBuffer->Map(false);
      if (pMapped) {
        memcpy(pMapped, drawable.drawable->data.data(),
               drawable.drawable->size());
      }
      transferBuffer->Unmap();
    }
    {
      px::CommandBuffer::CreateInfo info{};
      info.allocator = allocator;
      auto commandBuffer = device->AcquireCommandBuffer(info);
      {
        auto copyPass = commandBuffer->BeginCopyPass();
        {
          px::BufferTransferInfo src{};
          src.offset = 0;
          src.transferBuffer = transferBuffer;
          px::BufferRegion dst{};
          dst.offset = 0;
          dst.size = drawable.drawable->size();
          dst.buffer = instanceBuffer;
          copyPass->UploadBuffer(src, dst, false);
        }
        commandBuffer->EndCopyPass(copyPass);
      }
      device->SubmitCommandBuffer(commandBuffer);
    }

    drawable.vertexBuffers.emplace_back(px::BufferBinding{
        .buffer = GetModelData(draw3D.obj->model.data)->vertexBuffer,
        .offset = 0});
    drawable.indexBuffer = px::BufferBinding{
        .buffer = GetModelData(draw3D.obj->model.data)->indexBuffer,
        .offset = 0};
    drawable.vertexBuffers.emplace_back(
        px::BufferBinding{.buffer = instanceBuffer, .offset = 0

        });
    auto commandBuffer = currentCommandBuffer;
    auto renderPass = currentRenderPass;
    renderPass->BindGraphicsPipeline(currentPipeline3D.getInstanced());
    renderPass->BindFragmentSamplers(0, drawable.textureSamplers);
    renderPass->BindVertexBuffers(0, drawable.vertexBuffers);
    renderPass->BindIndexBuffer(drawable.indexBuffer,
                                px::IndexElementSize::Uint32);

    auto param = drawable.drawable->param;
    commandBuffer->PushVertexUniformData(0, &param,
                                         sizeof(Drawable::parameter));
    renderPass->DrawIndexedPrimitives(
        GetModelData(drawable.drawable->model.data)->v_array.indexCount,
        drawable.drawable->data.size(), 0, 0, 0);
  } else {
    drawable.vertexBuffers.emplace_back(px::BufferBinding{
        .buffer = GetModelData(draw3D.obj->model.data)->vertexBuffer,
        .offset = 0});
    drawable.indexBuffer = px::BufferBinding{
        .buffer = GetModelData(draw3D.obj->model.data)->indexBuffer,
        .offset = 0};
    auto commandBuffer = currentCommandBuffer;
    auto renderPass = currentRenderPass;
    renderPass->BindGraphicsPipeline(currentPipeline3D.get());
    renderPass->BindFragmentSamplers(0, drawable.textureSamplers);
    renderPass->BindVertexBuffers(0, drawable.vertexBuffers);
    renderPass->BindIndexBuffer(drawable.indexBuffer,
                                px::IndexElementSize::Uint32);

    auto param = drawable.drawable->param;
    commandBuffer->PushVertexUniformData(0, &param,
                                         sizeof(Drawable::parameter));
    renderPass->DrawIndexedPrimitives(
        GetModelData(drawable.drawable->model.data)->v_array.indexCount, 1, 0,
        0, 0);
  }
}
void GraphicsSystem::DrawRect(const Rect &rect, const Color &color,
                              float angle) {
  sinen::Draw2D draw2D;
  draw2D.position = rect.p;
  draw2D.scale = rect.s;

  draw2D.rotation = angle;
  draw2D.material = Material();
  Texture texture;
  texture.fillColor(color);
  draw2D.material.setTexture(texture);
  GraphicsSystem::Draw2D(draw2D);
}
void GraphicsSystem::DrawImage(const Texture &texture, const Rect &rect,
                               float angle) {
  sinen::Draw2D draw2D;
  draw2D.position = rect.p;
  draw2D.scale = rect.s;

  draw2D.rotation = angle;
  draw2D.material = Material();
  draw2D.material.setTexture(texture);
  GraphicsSystem::Draw2D(draw2D);
}
void GraphicsSystem::DrawText(const std::string &text,
                              const glm::vec2 &position, const Color &color,
                              float fontSize, float angle) {
  sinen::Draw2D draw2D;
  draw2D.position = position;
  Font font;
  font.load(fontSize);
  Texture texture;
  font.renderText(texture, text, color);
  draw2D.scale = texture.size();
  draw2D.rotation = angle;
  draw2D.material = Material();
  draw2D.material.setTexture(texture);
  GraphicsSystem::Draw2D(draw2D);
}
void GraphicsSystem::DrawModel(const Model &model, const Transform &transform,
                               const Material &material) {
  sinen::Draw3D draw3D;
  draw3D.position = transform.position;
  draw3D.scale = transform.scale;
  draw3D.rotation = transform.rotation;
  draw3D.material = material;
  draw3D.model = model;
  GraphicsSystem::Draw3D(draw3D);
}

void GraphicsSystem::load_shader(const Shader &shaderInfo) {}
void GraphicsSystem::unload_shader(const Shader &shaderInfo) {}
void *GraphicsSystem::get_texture_id() { return nullptr; }

void GraphicsSystem::setup_shapes() {
  box.loadFromVertexArray(create_box_vertices());
  sprite.loadFromVertexArray(create_sprite_vertices());
}

void GraphicsSystem::prepare_imgui() {
  ImGuiIO &io = ImGui::GetIO();
  io.WantTextInput = true;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
  // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.Fonts->AddFontFromMemoryTTF((void *)mplus_1p_medium_ttf,
                                 mplus_1p_medium_ttf_len, 18.0f, nullptr,
                                 io.Fonts->GetGlyphRangesJapanese());
}
void GraphicsSystem::bind_pipeline3d(const GraphicsPipeline3D &pipeline) {
  currentPipeline3D = pipeline;
}
void GraphicsSystem::bind_default_pipeline3d() {
  currentPipeline3D = pipeline3D;
}
void GraphicsSystem::bind_pipeline2d(const GraphicsPipeline2D &pipeline) {
  currentPipeline2D = pipeline;
}
void GraphicsSystem::bind_default_pipeline2d() {
  currentPipeline2D = pipeline2D;
}
void GraphicsSystem::set_uniform_data(uint32_t slot, const UniformData &data) {
  currentCommandBuffer->PushVertexUniformData(slot, data.data.data(),
                                              data.data.size() * sizeof(float));
}
void GraphicsSystem::SetRenderTarget(const RenderTexture &texture) {
  auto tex = texture.getTexture();
  auto depthTex = texture.getDepthStencil();
  currentCommandBuffer = device->AcquireCommandBuffer({allocator});
  currentColorTargets[0].loadOp = px::LoadOp::Clear;
  currentColorTargets[0].texture = tex;
  currentDepthStencilInfo.texture = depthTex;
  currentRenderPass = currentCommandBuffer->BeginRenderPass(
      currentColorTargets, currentDepthStencilInfo);
  currentRenderPass->SetViewport(
      px::Viewport{0, 0, (float)texture.width, (float)texture.height, 0, 1});
  currentRenderPass->SetScissor(0, 0, (float)texture.width,
                                (float)texture.height);
  isDefaultPipeline = false;
}
void GraphicsSystem::Flush() {
  currentCommandBuffer->EndRenderPass(currentRenderPass);
  device->SubmitCommandBuffer(currentCommandBuffer);
  device->WaitForGPUIdle();
  currentCommandBuffer = mainCommandBuffer;
}
Texture GraphicsSystem::ReadbackTexture(const RenderTexture &srcRenderTexture) {
  Texture out;
  auto tex = srcRenderTexture.getTexture();
  auto outTextureData = GetTexData(out.textureData);
  SDL_free(outTextureData->pSurface);
  outTextureData->pSurface = SDL_CreateSurface(
      srcRenderTexture.width, srcRenderTexture.height, SDL_PIXELFORMAT_RGBA32);
  px::Texture::CreateInfo info{};
  info.allocator = allocator;
  info.width = srcRenderTexture.width;
  info.height = srcRenderTexture.height;
  info.layerCountOrDepth = 1;
  info.format = px::TextureFormat::B8G8R8A8_UNORM;
  info.usage = px::TextureUsage::Sampler;
  info.numLevels = 1;
  info.sampleCount = px::SampleCount::x1;
  info.type = px::TextureType::Texture2D;
  outTextureData->texture = device->CreateTexture(info);

  // Copy
  px::TransferBuffer::CreateInfo info2{};
  info2.allocator = allocator;
  info2.size = srcRenderTexture.width * srcRenderTexture.height * 4;
  info2.usage = px::TransferBufferUsage::Download;
  auto transferBuffer = device->CreateTransferBuffer(info2);
  {
    px::CommandBuffer::CreateInfo info{};
    info.allocator = allocator;
    auto commandBuffer = device->AcquireCommandBuffer(info);
    {
      auto copyPass = commandBuffer->BeginCopyPass();
      {
        px::TextureLocation src{};
        src.texture = tex;
        src.layer = 0;
        src.mipLevel = 0;
        px::TextureLocation dst{};
        dst.texture = outTextureData->texture;
        dst.layer = 0;
        dst.mipLevel = 0;
        copyPass->CopyTexture(src, dst, srcRenderTexture.width,
                              srcRenderTexture.height, 1, false);
      }
      commandBuffer->EndCopyPass(copyPass);
    }
    device->SubmitCommandBuffer(commandBuffer);
    device->WaitForGPUIdle();
  }
  currentRenderPass = nullptr;
  return out;
}
} // namespace sinen
