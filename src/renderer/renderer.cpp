#include <renderer/renderer.hpp>

#include <core/logger/log.hpp>
#include <core/profiler.hpp>
#include <gpu/shader/builtin_shader.hpp>
#include <graphics/graphics.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <type_traits>

namespace sinen {
namespace {

#include <gpu/shader/default/renderer_npr.frag.snb.hpp>
#include <gpu/shader/default/renderer_pbr.frag.snb.hpp>
#include <gpu/shader/default/renderer_pbr_ibl.frag.snb.hpp>
#include <gpu/shader/default/renderer_vertex.vert.snb.hpp>

bool preparePipeline(Ptr<GraphicsPipeline> &pipeline,
                     const unsigned char *fragmentBundle, size_t bundleSize) {
  const auto sampleCount = Graphics::getMSAASampleCount();
  if (pipeline && pipeline->get() &&
      (1u << static_cast<UInt32>(
           pipeline->get()->getCreateInfo().multiSampleState.sampleCount)) ==
          sampleCount) {
    return true;
  }

  auto vertex = BuiltinShader::createFromBundle(renderer_vertex_vert_snb,
                                                renderer_vertex_vert_snb_len,
                                                ShaderStage::Vertex);
  auto fragment = BuiltinShader::createFromBundle(fragmentBundle, bundleSize,
                                                  ShaderStage::Fragment);
  if (!vertex.getRaw() || !fragment.getRaw()) {
    Log::error("Failed to create renderer shaders");
    return false;
  }

  pipeline = makePtr<GraphicsPipeline>(GlobalAllocator::get());
  pipeline->setVertexShader(vertex);
  pipeline->setFragmentShader(fragment);
  pipeline->setEnableDepthTest(true);
  pipeline->build();
  return pipeline->get() != nullptr;
}

template <class T> Buffer uniformBuffer(const T &value) {
  auto buffer = makeBuffer(sizeof(T), BufferType::Binary);
  std::memcpy(buffer.data(), &value, sizeof(T));
  return buffer;
}

struct LightUniform {
  float positionAndType[4];
  float directionAndIntensity[4];
  float colorAndInnerCos[4];
  float outerCos[4];
};
static_assert(sizeof(LightUniform) == 64);

LightUniform makeLightUniform(const Light &light) {
  return std::visit(
      [](const auto &value) {
        using T = std::decay_t<decltype(value)>;
        Vec3 position;
        Vec3 direction;
        float type = 0.0f;
        float innerCos = 1.0f;
        float outerCos = 1.0f;
        if constexpr (!std::is_same_v<T, DirectionalLight>) {
          position = value.getPosition();
          type = 1.0f;
        }
        if constexpr (!std::is_same_v<T, PointLight>) {
          direction = value.getDirection();
        }
        if constexpr (std::is_same_v<T, SpotLight>) {
          type = 2.0f;
          innerCos = std::cos(Math::toRadians(value.getInnerAngle()));
          outerCos = std::cos(Math::toRadians(value.getOuterAngle()));
        }
        const auto &color = value.getColor();
        return LightUniform{
            {position.x, position.y, position.z, type},
            {direction.x, direction.y, direction.z, value.getIntensity()},
            {color.r, color.g, color.b, innerCos},
            {outerCos, 0.0f, 0.0f, 0.0f},
        };
      },
      light);
}

bool hasTexture(const Ptr<Texture> &texture, gpu::TextureType type) {
  return texture && texture->getRaw() &&
         texture->getRaw()->getCreateInfo().type == type;
}

Ptr<Texture> solidTexture(const Color &color) {
  auto texture = Texture::create();
  texture->fill(color);
  return texture;
}

void bindTexture(Material &material, UInt32 slot, const Model &model,
                 TextureKey key, const Ptr<Texture> &fallback) {
  const auto texture = model.getTexture(key);
  material.setTexture(slot, hasTexture(texture, gpu::TextureType::Texture2D)
                                ? texture
                                : fallback);
}

} // namespace

void PBRRenderer::setLight(const Light &value) { light = value; }

void PBRRenderer::setPointLight(const Vec3 &position, float intensity,
                                const Color &color) {
  setLight(PointLight(position, intensity, color));
}
void PBRRenderer::setFillLight(const Light &value) { fillLight = value; }
void PBRRenderer::clearFillLight() { fillLight.reset(); }
void PBRRenderer::setExposure(float value) { exposure = std::max(0.0f, value); }

void PBRRenderer::setEnvironment(const Environment &value) {
  environment = value;
}

void PBRRenderer::clearEnvironment() { environment = {}; }

void PBRRenderer::drawModel(Render3DPass &pass, const Model &model,
                            const Transform &transform) {
  ZoneScopedN("PBRRenderer::drawModel");
  if (!model.vertexBuffer || !model.indexBuffer) {
    return;
  }

  const bool useIBL =
      hasTexture(environment.brdfLut, gpu::TextureType::Texture2D) &&
      hasTexture(environment.irradiance, gpu::TextureType::Cube) &&
      hasTexture(environment.prefiltered, gpu::TextureType::Cube);
  auto &pipeline = useIBL ? iblPipeline : directPipeline;
  const bool ready = useIBL
                         ? preparePipeline(pipeline, renderer_pbr_ibl_frag_snb,
                                           renderer_pbr_ibl_frag_snb_len)
                         : preparePipeline(pipeline, renderer_pbr_frag_snb,
                                           renderer_pbr_frag_snb_len);
  if (!ready) {
    return;
  }

  if (!white) {
    white = solidTexture(Color(1.0f));
    black = solidTexture(Color(0.0f));
    defaultMetallicRoughness = solidTexture(Color(0.0f, 1.0f, 0.0f, 1.0f));
  }

  Material material(pipeline);
  bindTexture(material, 0, model, TextureKey::BaseColor, white);
  bindTexture(material, 1, model, TextureKey::Normal, white);
  bindTexture(material, 2, model, TextureKey::DiffuseRoughness,
              defaultMetallicRoughness);
  bindTexture(material, 3, model, TextureKey::Emissive, black);
  bindTexture(material, 4, model, TextureKey::LightMap, white);

  float maxMip = 0.0f;
  if (useIBL) {
    material.setTexture(5, environment.brdfLut);
    material.setTexture(6, environment.irradiance);
    material.setTexture(7, environment.prefiltered);
    const auto levels =
        environment.prefiltered->getRaw()->getCreateInfo().numLevels;
    maxMip = static_cast<float>(std::max(levels, 1u) - 1);
  }

  const auto &cameraPosition = pass.getCamera().getPosition();
  const bool useNormalMap = hasTexture(model.getTexture(TextureKey::Normal),
                                       gpu::TextureType::Texture2D);
  const struct {
    float cameraPositionAndMaxMip[4];
    float settings[4];
    LightUniform light;
    LightUniform fillLight;
  } lighting = {
      {cameraPosition.x, cameraPosition.y, cameraPosition.z, maxMip},
      {useNormalMap ? 1.0f : 0.0f, exposure, fillLight ? 1.0f : 0.0f},
      makeLightUniform(light),
      fillLight ? makeLightUniform(*fillLight) : LightUniform{},
  };
  material.setUniformBuffer(0, uniformBuffer(lighting));
  material.setUniformBuffer(1, uniformBuffer(transform.getWorldMatrix()));
  pass.drawModel(model, transform, material);
}

void NPRRenderer::setLight(const Light &value) { light = value; }

void NPRRenderer::setLightDirection(const Vec3 &direction) {
  if (const auto *current = std::get_if<DirectionalLight>(&light)) {
    auto updated = *current;
    updated.setDirection(-1.0f * direction);
    setLight(updated);
  } else {
    setLight(DirectionalLight(-1.0f * direction));
  }
}

void NPRRenderer::setShadowColor(const Color &color) { shadowColor = color; }

void NPRRenderer::setBands(UInt32 value) { bands = std::max(2u, value); }

void NPRRenderer::drawModel(Render3DPass &pass, const Model &model,
                            const Transform &transform) {
  if (!model.vertexBuffer || !model.indexBuffer ||
      !preparePipeline(pipeline, renderer_npr_frag_snb,
                       renderer_npr_frag_snb_len)) {
    return;
  }

  const struct {
    float shadowColorAndBands[4];
    LightUniform light;
  } lighting = {
      {shadowColor.r, shadowColor.g, shadowColor.b, static_cast<float>(bands)},
      makeLightUniform(light),
  };
  Material material(pipeline);
  material.setUniformBuffer(0, uniformBuffer(lighting));
  material.setUniformBuffer(1, uniformBuffer(transform.getWorldMatrix()));
  pass.drawModel(model, transform, material);
}

} // namespace sinen
