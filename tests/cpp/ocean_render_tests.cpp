#define NOMINMAX
#include "gpu/backend/d3d12/d3d12_device.hpp"
#include "gpu/shader/shader_compiler_service.hpp"
#include "platform/window/window_native.hpp"
#include "script/bindings/luaapi.hpp"
#include "script/native_module.hpp"
#include <gpu/shader/shader_bundle.hpp>
#include <graphics/graphics.hpp>
#include <ocean/ocean.hpp>
#include <platform/window/window.hpp>

#include <SDL3/SDL.h>
#include <d3d12sdklayers.h>
#include <gtest/gtest.h>
#include <lua.h>
#include <lualib.h>
#include <stb_image.h>

#include <cstring>
#include <fstream>
#include <iterator>

using namespace sinen;
namespace sinen {
void registerGraphics(lua_State *);
void registerTransform(lua_State *);
} // namespace sinen
namespace {
class OceanRender : public testing::TestWithParam<GPUBackendAPI> {
protected:
  void SetUp() override {
    ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO));
    ASSERT_TRUE(Window::initialize("Ocean render verification"));
    SDL_HideWindow(static_cast<SDL_Window *>(WindowNative::getWindow()));
    ASSERT_TRUE(Graphics::initialize(GetParam()));
    if (GetParam() == GPUBackendAPI::D3D12)
      std::static_pointer_cast<gpu::d3d12::Device>(Graphics::getDevice())
          ->getNative()
          ->QueryInterface(IID_PPV_ARGS(&messages));
    if (messages)
      messages->ClearStoredMessages();
    state = luaL_newstate();
    luaL_openlibs(state);
    ensureSn(state);
    lua_pop(state, 1);
    registerGraphics(state);
    registerTransform(state);
    ASSERT_TRUE(nativeModuleManager().loadPlugin(state, "shader_compiler"));
    void *symbol = nativeModuleManager().lastPluginSymbol(
        SINEN_SHADER_COMPILER_PLUGIN_API_SYMBOL);
    ASSERT_NE(symbol, nullptr);
    sinen_shader_compiler_get_api_fn getApi = nullptr;
    std::memcpy(&getApi, &symbol, sizeof(getApi));
    std::string error;
    ASSERT_TRUE(registerShaderCompilerPlugin(getApi(), error)) << error;
  }
  void TearDown() override {
    if (Graphics::getDevice())
      Graphics::getDevice()->waitForGpuIdle();
    if (state) {
      nativeModuleManager().close(state);
      lua_close(state);
      state = nullptr;
    }
    Graphics::shutdown();
    if (messages) {
      for (UINT64 i = 0; i < messages->GetNumStoredMessages(); ++i) {
        SIZE_T size = 0;
        messages->GetMessage(i, nullptr, &size);
        std::vector<char> bytes(size);
        auto *message = reinterpret_cast<D3D12_MESSAGE *>(bytes.data());
        if (SUCCEEDED(messages->GetMessage(i, message, &size)) &&
            message->Severity <= D3D12_MESSAGE_SEVERITY_ERROR)
          ADD_FAILURE() << message->pDescription;
      }
    }
    messages.Reset();
    unregisterShaderCompilerPlugin();
    nativeModuleManager().unload();
    Window::shutdown();
    SDL_Quit();
  }
  Shader compile(ShaderStage stage) {
    const std::string path = SINEN_TEST_OCEAN_SOURCE;
    std::ifstream file(path);
    const std::string source((std::istreambuf_iterator<char>(file)), {});
    CompiledShader code;
    std::string error;
    const auto format = GetParam() == GPUBackendAPI::D3D12
                            ? ShaderFormat::DXIL
                            : ShaderFormat::SPIRV;
    if (!compileShaderSource("ocean", path, source, stage, format, code,
                             error)) {
      ADD_FAILURE() << error;
      return Shader{};
    }
    gpu::Shader::CreateInfo info{};
    info.allocator = GlobalAllocator::get();
    info.size = code.code.size();
    info.data = code.code.data();
    info.entrypoint = ShaderBundle::entryPointFor(stage, format);
    info.format = format;
    info.stage = stage;
    info.numUniformBuffers = code.numUniformBuffers;
    info.numSamplers = code.numCombinedSamplers;
    return Shader(Graphics::getDevice()->createShader(info));
  }
  lua_State *state = nullptr;
  gpu::d3d12::ComPtr<ID3D12InfoQueue> messages;
};
} // namespace

TEST_P(OceanRender, StreamsAnimatedMeshAndRendersWaterShader) {
  auto pipeline = makePtr<GraphicsPipeline>();
  auto vs = compile(ShaderStage::Vertex), fs = compile(ShaderStage::Fragment);
  ASSERT_NE(vs.getRaw(), nullptr);
  ASSERT_NE(fs.getRaw(), nullptr);
  pipeline->setVertexShader(vs);
  pipeline->setFragmentShader(fs);
  pipeline->setEnableDepthTest(true);
  pipeline->build();
  ASSERT_NE(pipeline->get(), nullptr);
  auto material = makePtr<Material>(pipeline);
  auto white = Texture::create(1, 1);
  white->fill(Color(1));
  material->setTexture(0, white);
  const Transform transform;
  auto world = transform.getWorldMatrix();
  auto worldBuffer = makeBuffer(sizeof(world));
  std::memcpy(worldBuffer.data(), &world, sizeof(world));
  material->setUniformBuffer(1, worldBuffer);
  const Vec4 eye(128, 8, 190, 0);
  auto eyeBuffer = makeBuffer(sizeof(eye));
  std::memcpy(eyeBuffer.data(), &eye, sizeof(eye));
  material->setUniformBuffer(0, eyeBuffer);
  Camera3D camera;
  camera.perspective(60, 16.0f / 9, 0.1f, 1200);
  camera.lookat(Vec3(eye.x, eye.y, eye.z), Vec3(128, 1, 60), Vec3(0, 1, 0));
  RenderTexture target;
  target.create(640, 360);
  auto image = Texture::create(640, 360);
  ocean::Settings settings;
  settings.seed = 42;
  ocean::Ocean simulation(settings);
  std::vector<ocean::Vertex> vertices(simulation.vertexCount());
  auto *vertexData =
      lua_newbuffer(state, vertices.size() * sizeof(ocean::Vertex));
  lua_setglobal(state, "vertices");
  auto *indexData = static_cast<uint32_t *>(
      lua_newbuffer(state, simulation.indexCount() * sizeof(uint32_t)));
  simulation.writeIndices({indexData, simulation.indexCount()});
  lua_setglobal(state, "indices");
  udNewOwned(state, transform);
  lua_setglobal(state, "transform");
  udPushPtr(state, material);
  lua_setglobal(state, "material");
  const auto script = Luau::compile(R"(
    local validated = false
    function drawOcean(pass)
      if not validated then
        assert(not pcall(function() pass:drawMesh(buffer.create(1), indices, transform, material) end))
        local badIndex = buffer.create(12)
        buffer.writeu32(badIndex, 0, 0xffffffff)
        assert(not pcall(function() pass:drawMesh(vertices, badIndex, transform, material) end))
        local badVertex = buffer.create(48)
        buffer.writef32(badVertex, 0, 0/0)
        assert(not pcall(function() pass:drawMesh(badVertex, buffer.create(12), transform, material) end))
        validated = true
      end
      pass:drawMesh(vertices, indices, transform, material)
    end
  )");
  ASSERT_EQ(luau_load(state, "ocean_render", script.data(), script.size(), 0),
            LUA_OK);
  ASSERT_EQ(lua_pcall(state, 0, 0, 0), LUA_OK) << lua_tostring(state, -1);
  Graphics::setClearColor(Color(1, 0, 1, 1));
  Graphics::addPostDrawFunc([&] {
    auto pass = Graphics::begin3DPass(camera, target);
    lua_getglobal(state, "drawOcean");
    udPushPtr(state, pass);
    EXPECT_EQ(lua_pcall(state, 1, 0, 0), LUA_OK) << lua_tostring(state, -1);
    Graphics::endPass(pass);
  });
  for (int frame = 0; frame < 3; ++frame) {
    simulation.update(2.0 + frame * 0.2);
    simulation.writeVertices(vertices);
    std::memcpy(vertexData, vertices.data(),
                vertices.size() * sizeof(ocean::Vertex));
    Graphics::render();
    ASSERT_TRUE(Graphics::readbackTexture(target, image));
  }
  const auto png = image->toPngBuffer();
  int width = 0, height = 0, channels = 0;
  auto *pixels =
      stbi_load_from_memory(static_cast<const stbi_uc *>(png.data()),
                            png.size(), &width, &height, &channels, 4);
  ASSERT_NE(pixels, nullptr);
  int waterPixels = 0;
  for (int i = 0; i < width * height; ++i)
    if (pixels[i * 4 + 1] > 20 && pixels[i * 4 + 2] > pixels[i * 4])
      ++waterPixels;
  EXPECT_GT(waterPixels, width * height / 3);
  stbi_image_free(pixels);
  if (GetParam() == GPUBackendAPI::D3D12) {
    std::ofstream output("ocean-preview.png", std::ios::binary);
    output.write(static_cast<const char *>(png.data()), png.size());
  }
}

INSTANTIATE_TEST_SUITE_P(Backends, OceanRender,
                         testing::Values(GPUBackendAPI::D3D12,
                                         GPUBackendAPI::Vulkan));
