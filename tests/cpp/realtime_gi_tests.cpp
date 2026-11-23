#define NOMINMAX
#include "gpu/backend/d3d12/d3d12_device.hpp"
#include "platform/window/window_native.hpp"
#include <SDL3/SDL.h>
#include <cstring>
#include <d3d12sdklayers.h>
#include <filesystem>
#include <fstream>
#include <graphics/graphics.hpp>
#include <graphics/model/mesh_builder.hpp>
#include <gtest/gtest.h>
#include <limits>
#include <platform/window/window.hpp>
#include <renderer/global_illumination.hpp>
#include <renderer/pbr_renderer.hpp>
#include <stb_image.h>

using namespace sinen;
namespace {
class RealtimeGITests : public testing::TestWithParam<GPUBackendAPI> {
protected:
  void SetUp() override {
    ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO));
    ASSERT_TRUE(Window::initialize("Sinen real-time GI test"));
    SDL_HideWindow(static_cast<SDL_Window *>(WindowNative::getWindow()));
    ASSERT_TRUE(Graphics::initialize(GetParam()));
    device = Graphics::getDevice();
    if (GetParam() == GPUBackendAPI::D3D12)
      std::static_pointer_cast<gpu::d3d12::Device>(device)
          ->getNative()
          ->QueryInterface(IID_PPV_ARGS(&messages));
    if (messages)
      messages->ClearStoredMessages();
  }
  void TearDown() override {
    if (device)
      device->waitForGpuIdle();
    Graphics::shutdown();
    if (messages)
      for (UINT64 i = 0; i < messages->GetNumStoredMessages(); ++i) {
        SIZE_T size = 0;
        messages->GetMessage(i, nullptr, &size);
        std::vector<char> data(size);
        auto *message = reinterpret_cast<D3D12_MESSAGE *>(data.data());
        if (SUCCEEDED(messages->GetMessage(i, message, &size)) &&
            message->Severity <= D3D12_MESSAGE_SEVERITY_ERROR)
          ADD_FAILURE() << message->pDescription;
      }
    device.reset();
    messages.Reset();
    Window::shutdown();
    SDL_Quit();
  }
  Vec3 readMean(const GlobalIllumination &gi) {
    auto texture = gi.getTexture()->getRaw();
    const auto info = texture->getCreateInfo();
    const UInt32 row = GetParam() == GPUBackendAPI::Vulkan
                           ? info.width * 16
                           : (info.width * 16 + 255) & ~255u;
    auto download = device->createTransferBuffer(
        {GlobalAllocator::get(), gpu::TransferBufferUsage::Download,
         row * info.height});
    auto commands = device->acquireCommandBuffer({GlobalAllocator::get()});
    auto copy = commands->beginCopyPass();
    gpu::TextureRegion region{};
    region.texture = texture;
    region.width = info.width;
    region.height = info.height;
    region.depth = 1;
    copy->downloadTexture(region, {download, 0});
    commands->endCopyPass(copy);
    device->submitCommandBuffer(commands);
    device->waitForGpuIdle();
    const auto *bytes = static_cast<const char *>(download->map(false));
    if (!bytes) {
      ADD_FAILURE() << "GI atlas readback failed";
      return Vec3(0);
    }
    Vec3 result;
    for (UInt32 y = 0; y < info.height; ++y)
      for (UInt32 x = 0; x < info.width; ++x) {
        float rgba[4];
        std::memcpy(rgba, bytes + y * row + x * 16, sizeof(rgba));
        result += Vec3(rgba[0], rgba[1], rgba[2]);
      }
    download->unmap();
    return result * (1.0f / (info.width * info.height));
  }
  Model box() {
    MeshBuilder builder;
    builder.addBox(Vec3(4));
    Model model;
    *model.getMesh().data() = *builder.toMesh().data();
    return model;
  }
  Ptr<gpu::Device> device;
  gpu::d3d12::ComPtr<ID3D12InfoQueue> messages;
};
} // namespace

TEST_P(RealtimeGITests, UpdatesSkyOnGpuWithoutReinitializingTheVolume) {
  GlobalIllumination gi;
  EXPECT_FALSE(gi.initializeRealtime(Vec3(1), Vec3(-1)));
  EXPECT_FALSE(gi.initializeRealtime(Vec3(-1), Vec3(1), 1));
  gi.setSkyColor(Color(0.2f, 0.4f, 0.8f, 1.0f));
  ASSERT_TRUE(gi.initializeRealtime(Vec3(-1), Vec3(1), 2, 2, 2));
  ASSERT_TRUE(gi.isRealtime());
  const auto atlas = gi.getTexture();
  ASSERT_TRUE(gi.update(0));
  const auto first = readMean(gi);
  EXPECT_NEAR(first.x, 0.2f * 3.14159f, 0.06f);
  EXPECT_NEAR(first.y, 0.4f * 3.14159f, 0.12f);
  EXPECT_NEAR(first.z, 0.8f * 3.14159f, 0.24f);
  gi.setSkyColor(Color(0.8f, 0.1f, 0.05f, 1.0f));
  ASSERT_TRUE(gi.update(0));
  const auto second = readMean(gi);
  EXPECT_EQ(gi.getTexture(), atlas);
  EXPECT_GT(second.x, first.x * 3);
  EXPECT_LT(second.z, first.z * 0.1f);
  EXPECT_FALSE(gi.update(std::numeric_limits<float>::quiet_NaN()));
  gi.clear();
  EXPECT_FALSE(gi.isRealtime());
  EXPECT_FALSE(gi.update());
}

TEST_P(RealtimeGITests, MovingGeometryChangesEmissionAndOcclusionNextFrame) {
  GlobalIllumination gi;
  gi.setSkyColor(Color(0, 0, 1, 1));
  auto model = box();
  gi.addModel(model, Transform{}, Color(0), Color(1, 0, 0, 1));
  ASSERT_TRUE(gi.initializeRealtime(Vec3(-1), Vec3(1), 2, 2, 2));
  ASSERT_TRUE(gi.update(0));
  const auto inside = readMean(gi);
  EXPECT_GT(inside.x, 2.5f);
  EXPECT_FLOAT_EQ(inside.z, 0.0f);
  Transform moved;
  moved.position = Vec3(10, 0, 0);
  gi.beginFrame();
  gi.addModel(model, moved, Color(0), Color(1, 0, 0, 1));
  ASSERT_TRUE(gi.update(0));
  const auto outside = readMean(gi);
  EXPECT_LT(outside.x, 0.2f);
  EXPECT_GT(outside.z, 2.5f);
}

TEST_P(RealtimeGITests, MovingLightChangesReflectedLightAndShadows) {
  GlobalIllumination gi;
  gi.addModel(box(), Transform{});
  gi.setLight(PointLight(Vec3(0), 1));
  ASSERT_TRUE(gi.initializeRealtime(Vec3(-1), Vec3(1), 2, 2, 2));
  ASSERT_TRUE(gi.update(0));
  const auto inside = readMean(gi);
  EXPECT_GT(inside.x, 0.1f);
  gi.setLight(PointLight(Vec3(10, 0, 0), 1));
  ASSERT_TRUE(gi.update(0));
  const auto outside = readMean(gi);
  EXPECT_LT(outside.x, inside.x * 0.01f);
}

TEST_P(RealtimeGITests, BlendsHistoryAndCanRespondImmediately) {
  GlobalIllumination gi;
  ASSERT_TRUE(gi.initializeRealtime(Vec3(-1), Vec3(1), 2, 2, 2));
  ASSERT_TRUE(gi.update());
  EXPECT_FLOAT_EQ(readMean(gi).x, 0.0f);
  gi.setSkyColor(Color(1));
  ASSERT_TRUE(gi.update(0.5f));
  const auto smoothed = readMean(gi);
  EXPECT_NEAR(smoothed.x, 3.14159f * 0.5f, 0.12f);
  ASSERT_TRUE(gi.update(0));
  const auto immediate = readMean(gi);
  EXPECT_NEAR(immediate.x, 3.14159f, 0.24f);
  EXPECT_GT(immediate.x, smoothed.x * 1.8f);
}

TEST_P(RealtimeGITests, RejectsInvalidGpuBufferTextureCopies) {
  auto buffer = device->createBuffer(
      {GlobalAllocator::get(), gpu::BufferUsage::Storage, 512});
  auto texture = Texture::create(2, 2);
  auto commands = device->acquireCommandBuffer({GlobalAllocator::get()});
  auto copy = commands->beginCopyPass();
  gpu::TextureRegion region{};
  region.texture = texture->getRaw();
  region.width = 2;
  region.height = 2;
  region.depth = 1;
  EXPECT_FALSE(copy->copyBufferToTexture({buffer, 1, 511}, region, 256));
  EXPECT_FALSE(copy->copyBufferToTexture({buffer, 0, 512}, region, 255));
  EXPECT_FALSE(copy->copyBufferToTexture({buffer, 0, 1}, region, 256));
  EXPECT_FALSE(copy->copyBufferToTexture({nullptr, 0, 512}, region, 256));
  region.depth = 2;
  EXPECT_FALSE(copy->copyBufferToTexture({buffer, 0, 512}, region, 256));
  commands->endCopyPass(copy);
  device->submitCommandBuffer(commands);
}

TEST_P(RealtimeGITests, ImportedProxyPreservesEveryVertexColor) {
  constexpr char source[] =
      "ply\nformat ascii 1.0\nelement vertex 3\n"
      "property float x\nproperty float y\nproperty float z\n"
      "property float nx\nproperty float ny\nproperty float nz\n"
      "property float s\nproperty float t\n"
      "property uchar red\nproperty uchar green\nproperty uchar blue\nproperty "
      "uchar alpha\n"
      "element face 1\nproperty list uchar int vertex_indices\nend_header\n"
      "0 0 0 0 0 1 0 0 255 0 0 255\n"
      "1 0 0 0 0 1 1 0 0 255 0 255\n"
      "0 1 0 0 0 1 0 1 0 0 255 255\n3 0 1 2\n";
  auto buffer = makeBuffer(sizeof(source) - 1, BufferType::Binary);
  std::memcpy(buffer.data(), source, sizeof(source) - 1);
  Model model;
  model.load(buffer, "ply");
  const auto timeout = SDL_GetTicks() + 5000;
  while (!model.vertexBuffer && SDL_GetTicks() < timeout)
    Graphics::render();
  ASSERT_NE(model.vertexBuffer, nullptr);
  const auto mesh = model.getMesh().data();
  ASSERT_EQ(mesh->vertices.size(), 3u);
  for (const auto &vertex : mesh->vertices) {
    EXPECT_FLOAT_EQ(vertex.color.r,
                    vertex.position.x == 0 && vertex.position.y == 0 ? 1.0f
                                                                     : 0.0f);
    EXPECT_FLOAT_EQ(vertex.color.g, vertex.position.x == 1 ? 1.0f : 0.0f);
    EXPECT_FLOAT_EQ(vertex.color.b, vertex.position.y == 1 ? 1.0f : 0.0f);
  }
}

TEST_P(RealtimeGITests, PbrAlphaCutoffDiscardsTransparentSurfaces) {
  Graphics::setMSAASampleCount(1);
  MeshBuilder builder;
  builder.addBox(Vec3(1));
  Model model;
  model.loadFromVertexArray(builder.toMesh(), false);
  auto transparent = Texture::create();
  transparent->fill(Color(1, 0, 0, 0));
  model.setTexture(TextureKey::BaseColor, transparent);
  PBRRenderer renderer;
  renderer.setPointLight(Vec3(0, 0, 3), 0);
  Camera3D camera;
  camera.lookat(Vec3(0, 0, 3), Vec3(0), Vec3(0, 1, 0));
  camera.perspective(45, 1, 0.1f, 10);
  RenderTexture target;
  target.create(32, 32);
  auto output = Texture::create(32, 32);
  int stage = 0;
  int red[2]{};
  Graphics::setClearColor(Color(0));
  Graphics::addPostDrawFunc([&] {
    if (stage >= 2)
      return;
    renderer.setAlphaCutoff(stage == 0 ? 0.0f : 0.5f);
    auto pass = Graphics::begin3DPass(camera, target);
    ASSERT_NE(pass, nullptr);
    renderer.drawModel(*pass, model, Transform{});
    Graphics::endPass(pass);
    ASSERT_TRUE(Graphics::readbackTexture(target, output));
    const auto png = output->toPngBuffer();
    int w = 0, h = 0, channels = 0;
    auto *pixels = stbi_load_from_memory(
        static_cast<const unsigned char *>(png.data()),
        static_cast<int>(png.size()), &w, &h, &channels, 4);
    ASSERT_NE(pixels, nullptr);
    red[stage] = pixels[((h / 2) * w + w / 2) * 4];
    stbi_image_free(pixels);
    ++stage;
  });
  Graphics::render();
  Graphics::render();
  ASSERT_EQ(stage, 2);
  EXPECT_GT(red[0], 70);
  EXPECT_LT(red[1], 5);
}

const GPUBackendAPI giTestBackends[] = {GPUBackendAPI::D3D12,
                                        GPUBackendAPI::Vulkan
#ifdef SINEN_ENABLE_WEBGPU
                                        ,
                                        GPUBackendAPI::WebGPU
#endif
};
INSTANTIATE_TEST_SUITE_P(Backends, RealtimeGITests,
                         testing::ValuesIn(giTestBackends));

TEST_P(RealtimeGITests,
       ImportsAllMaterialsAndRepeatedNodesWithWorldTransforms) {
  constexpr char source[] =
      R"gltf({"asset":{"version":"2.0"},"buffers":[{"byteLength":42,"uri":"data:application/octet-stream;base64,AAAAAAAAAAAAAAAAAACAPwAAAAAAAAAAAAAAAAAAgD8AAAAAAAABAAIA"}],"bufferViews":[{"buffer":0,"byteOffset":0,"byteLength":36},{"buffer":0,"byteOffset":36,"byteLength":6}],"accessors":[{"bufferView":0,"componentType":5126,"count":3,"type":"VEC3","min":[0,0,0],"max":[1,1,0]},{"bufferView":1,"componentType":5123,"count":3,"type":"SCALAR"}],"materials":[{"name":"red","pbrMetallicRoughness":{"baseColorFactor":[1,0,0,1],"metallicFactor":0.25,"roughnessFactor":0.75}},{"name":"green","pbrMetallicRoughness":{"baseColorFactor":[0,1,0,1]}}],"meshes":[{"primitives":[{"attributes":{"POSITION":0},"indices":1,"material":0}]},{"primitives":[{"attributes":{"POSITION":0},"indices":1,"material":1}]}],"nodes":[{"translation":[2,3,4],"children":[1,2,3]},{"mesh":0},{"mesh":1,"translation":[2,0,0],"scale":[-1,2,1]},{"mesh":0,"translation":[4,0,0]}],"scenes":[{"nodes":[0]}],"scene":0})gltf";
  auto bytes = makeBuffer(sizeof(source) - 1, BufferType::Binary);
  std::memcpy(bytes.data(), source, sizeof(source) - 1);
  Model model;
  model.load(bytes, "gltf");
  const auto timeout = SDL_GetTicks() + 5000;
  while (!model.isReady() && SDL_GetTicks() < timeout)
    Graphics::render();
  ASSERT_TRUE(model.isReady());
  ASSERT_EQ(model.getParts().size(), 3u);
  EXPECT_EQ(model.getMaterialCount(), 2u);
  EXPECT_EQ(model.getTriangleCount(), 3u);
  EXPECT_FLOAT_EQ(model.getAABB().min.x, 2);
  EXPECT_FLOAT_EQ(model.getAABB().min.y, 3);
  EXPECT_FLOAT_EQ(model.getAABB().min.z, 4);
  EXPECT_FLOAT_EQ(model.getAABB().max.x, 7);
  EXPECT_FLOAT_EQ(model.getAABB().max.y, 5);
  bool green = false;
  for (const auto &part : model.getParts()) {
    ASSERT_NE(part->vertexBuffer, nullptr);
    const auto data = part->getMesh().data();
    ASSERT_EQ(data->vertices.size(), 3u);
    const auto &surface = part->getSurfaceProperties();
    if (surface.materialName == "green") {
      green = true;
      EXPECT_FLOAT_EQ(data->vertices[0].color.g, 1);
      const auto &a = data->vertices[data->indices[0]];
      const auto &b = data->vertices[data->indices[1]];
      const auto &c = data->vertices[data->indices[2]];
      EXPECT_GT(Vec3::dot(Vec3::cross(b.position - a.position,
                                      c.position - a.position),
                          a.normal),
                0);
    } else {
      EXPECT_EQ(surface.materialName, "red");
      EXPECT_FLOAT_EQ(data->vertices[0].color.r, 1);
      EXPECT_NEAR(surface.metallicFactor, 0.25f, 1e-6f);
      EXPECT_NEAR(surface.roughnessFactor, 0.75f, 1e-6f);
    }
  }
  EXPECT_TRUE(green);
  EXPECT_EQ(model.getMesh().data()->indices.size(), 9u);
  auto override = Texture::create();
  override->fill(Color(0, 0, 1, 1));
  model.setTexture(TextureKey::BaseColor, override);
  for (const auto &part : model.getParts())
    EXPECT_EQ(part->getTexture(TextureKey::BaseColor), override);
  model.loadBox();
  EXPECT_TRUE(model.getParts().empty());
  EXPECT_EQ(model.getTriangleCount(), 12u);
}

TEST_P(RealtimeGITests, LoadsSharedExternalDdsAndDrawsEveryMaterial) {
  const auto directory =
      std::filesystem::path(SDL_GetBasePath()) / "model-import-fixture";
  std::filesystem::create_directories(directory / "Textures");
  auto write = [&](const char *name, const std::string &text) {
    std::ofstream file(directory / name, std::ios::binary);
    file.write(text.data(), text.size());
  };
  std::string dds(144, 0);
  auto word = [&](size_t offset, uint32_t value) {
    for (int i = 0; i < 4; ++i)
      dds[offset + i] = char(value >> (i * 8));
  };
  dds.replace(0, 4, "DDS ");
  word(4, 124);
  word(12, 4);
  word(16, 4);
  word(28, 2);
  word(76, 32);
  word(80, 4);
  dds.replace(84, 4, "DXT1");
  dds[128] = dds[129] = dds[136] = dds[137] = char(255);
  write("Textures/white.dds", dds);
  write("scene.mtl", "newmtl red\nKd 1 0 0\nmap_Kd Textures/white.dds\n"
                     "newmtl green\nKd 0 1 0\nmap_Kd Textures/white.dds\n");
  write("scene.obj", "mtllib scene.mtl\n"
                     "v -1.8 -1 0\nv -0.2 -1 0\nv -1 1 0\n"
                     "v 0.2 -1 0\nv 1.8 -1 0\nv 1 1 0\n"
                     "usemtl red\nf 1 2 3\nusemtl green\nf 4 5 6\n");
  Model model;
  Model::LoadOptions options;
  options.maxTextureSize = 2;
  model.load("model-import-fixture/scene.obj", options);
  const auto timeout = SDL_GetTicks() + 5000;
  while (!model.isReady() && SDL_GetTicks() < timeout)
    Graphics::render();
  ASSERT_TRUE(model.isReady());
  ASSERT_EQ(model.getParts().size(), 2u);
  const auto first = model.getParts()[0]->getTexture(TextureKey::BaseColor);
  const auto second = model.getParts()[1]->getTexture(TextureKey::BaseColor);
  ASSERT_NE(first, nullptr);
  EXPECT_EQ(first, second);
  ASSERT_NE(first->getRaw(), nullptr);
  EXPECT_EQ(first->getRaw()->getCreateInfo().width, 2u);
  EXPECT_FALSE(first->hasTransparency());
  EXPECT_NEAR(first->averageColor().r, 1, 0.01f);

  Graphics::setMSAASampleCount(1);
  Graphics::setClearColor(Color(0));
  PBRRenderer renderer;
  renderer.setPointLight(Vec3(0, 0, 4), 0);
  Camera3D camera;
  camera.lookat(Vec3(0, 0, 4), Vec3(0), Vec3(0, 1, 0));
  camera.perspective(45, 2, 0.1f, 20);
  RenderTexture target;
  target.create(64, 32);
  auto output = Texture::create(64, 32);
  bool rendered = false;
  Graphics::addPostDrawFunc([&] {
    if (rendered)
      return;
    auto pass = Graphics::begin3DPass(camera, target);
    ASSERT_NE(pass, nullptr);
    renderer.drawModel(*pass, model, Transform{});
    Graphics::endPass(pass);
    ASSERT_TRUE(Graphics::readbackTexture(target, output));
    const auto png = output->toPngBuffer();
    int w, h, channels;
    auto *pixels = stbi_load_from_memory(
        static_cast<const unsigned char *>(png.data()),
        static_cast<int>(png.size()), &w, &h, &channels, 4);
    ASSERT_NE(pixels, nullptr);
    int reds = 0, greens = 0;
    for (int i = 0; i < w * h; ++i) {
      if (pixels[i * 4] > 70 && pixels[i * 4 + 1] < 5)
        ++reds;
      if (pixels[i * 4 + 1] > 70 && pixels[i * 4] < 5)
        ++greens;
    }
    EXPECT_GT(reds, 20);
    EXPECT_GT(greens, 20);
    stbi_image_free(pixels);
    rendered = true;
  });
  Graphics::render();
  EXPECT_TRUE(rendered);
}
