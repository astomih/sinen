#include <gpu/shader/builtin_shader.hpp>
#include <graphics/graphics.hpp>
#include <renderer/global_illumination.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <limits>
#include <meshoptimizer.h>
#include <numeric>
#include <type_traits>

namespace sinen {
namespace {
#include <gpu/shader/default/renderer_gi.comp.snb.hpp>
constexpr float pi = 3.14159265358979323846f;
constexpr float epsilon = 0.0001f;
const std::array<Vec3, 6> axes = {Vec3(1, 0, 0), Vec3(-1, 0, 0),
                                  Vec3(0, 1, 0), Vec3(0, -1, 0),
                                  Vec3(0, 0, 1), Vec3(0, 0, -1)};
float component(const Vec3 &v, int i) {
  return i == 0 ? v.x : i == 1 ? v.y : v.z;
}
bool finite(const Vec3 &v) {
  return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}
Vec3 rgb(const Color &c) {
  return Vec3(std::isfinite(c.r) ? std::max(c.r, 0.0f) : 0.0f,
              std::isfinite(c.g) ? std::max(c.g, 0.0f) : 0.0f,
              std::isfinite(c.b) ? std::max(c.b, 0.0f) : 0.0f);
}
// Integer RNG with a fixed seed makes bakes reproducible across runs.
float random01(uint32_t &state) {
  state ^= state << 13;
  state ^= state >> 17;
  state ^= state << 5;
  return static_cast<float>(state >> 8) * (1.0f / 16777216.0f);
}
Vec3 cosineDirection(const Vec3 &normal, uint32_t &state) {
  const float r = std::sqrt(random01(state));
  const float phi = 2.0f * pi * random01(state);
  const Vec3 tangent = Vec3::normalize(Vec3::cross(
      std::abs(normal.y) < 0.9f ? Vec3(0, 1, 0) : Vec3(1, 0, 0), normal));
  return tangent * (r * std::cos(phi)) +
         Vec3::cross(normal, tangent) * (r * std::sin(phi)) +
         normal * std::sqrt(std::max(0.0f, 1.0f - r * r));
}

template <class Triangle> class RayScene {
  struct Node {
    Vec3 lo, hi;
    size_t begin = 0, end = 0;
    int left = -1, right = -1;
  };
  const std::vector<Triangle> &triangles;
  std::vector<size_t> order;
  std::vector<Node> nodes;

  int build(size_t begin, size_t end) {
    const float inf = std::numeric_limits<float>::infinity();
    Node node{Vec3(inf), Vec3(-inf), begin, end};
    for (size_t i = begin; i < end; ++i) {
      const auto &t = triangles[order[i]];
      for (const Vec3 &p : {t.a, t.b, t.c}) {
        node.lo = Vec3(std::min(node.lo.x, p.x), std::min(node.lo.y, p.y),
                       std::min(node.lo.z, p.z));
        node.hi = Vec3(std::max(node.hi.x, p.x), std::max(node.hi.y, p.y),
                       std::max(node.hi.z, p.z));
      }
    }
    const int index = static_cast<int>(nodes.size());
    nodes.push_back(node);
    if (end - begin > 4) {
      const Vec3 extent = node.hi - node.lo;
      const int axis = extent.x > extent.y ? (extent.x > extent.z ? 0 : 2)
                                           : (extent.y > extent.z ? 1 : 2);
      const size_t middle = (begin + end) / 2;
      std::nth_element(order.begin() + begin, order.begin() + middle,
                       order.begin() + end, [&](size_t a, size_t b) {
                         const auto &ta = triangles[a];
                         const auto &tb = triangles[b];
                         return component(ta.a + ta.b + ta.c, axis) <
                                component(tb.a + tb.b + tb.c, axis);
                       });
      nodes[index].left = build(begin, middle);
      nodes[index].right = build(middle, end);
    }
    return index;
  }
  bool box(const Node &node, const Vec3 &origin, const Vec3 &direction,
           float limit) const {
    float near = 0.0f;
    for (int axis = 0; axis < 3; ++axis) {
      const float o = component(origin, axis), d = component(direction, axis);
      const float lo = component(node.lo, axis), hi = component(node.hi, axis);
      if (std::abs(d) < 1e-12f) {
        if (o < lo || o > hi)
          return false;
      } else {
        float a = (lo - o) / d, b = (hi - o) / d;
        if (a > b)
          std::swap(a, b);
        near = std::max(near, a);
        limit = std::min(limit, b);
        if (near > limit)
          return false;
      }
    }
    return true;
  }
  void intersectNode(int index, const Vec3 &origin, const Vec3 &direction,
                     float &distance, const Triangle *&hit) const {
    const auto &node = nodes[index];
    if (!box(node, origin, direction, distance))
      return;
    if (node.left >= 0) {
      intersectNode(node.left, origin, direction, distance, hit);
      intersectNode(node.right, origin, direction, distance, hit);
      return;
    }
    for (size_t i = node.begin; i < node.end; ++i) {
      const auto &t = triangles[order[i]];
      const Vec3 edge1 = t.b - t.a, edge2 = t.c - t.a;
      const Vec3 p = Vec3::cross(direction, edge2);
      const float det = Vec3::dot(edge1, p);
      if (std::abs(det) < 1e-9f)
        continue;
      const Vec3 offset = origin - t.a;
      const float u = Vec3::dot(offset, p) / det;
      if (u < 0.0f || u > 1.0f)
        continue;
      const Vec3 q = Vec3::cross(offset, edge1);
      const float v = Vec3::dot(direction, q) / det;
      if (v < 0.0f || u + v > 1.0f)
        continue;
      const float d = Vec3::dot(edge2, q) / det;
      if (d > epsilon && d < distance) {
        distance = d;
        hit = &t;
      }
    }
  }

public:
  explicit RayScene(const std::vector<Triangle> &triangles)
      : triangles(triangles) {
    order.resize(triangles.size());
    std::iota(order.begin(), order.end(), size_t(0));
    if (!order.empty())
      build(0, order.size());
  }
  std::vector<uint32_t> pack() const {
    // All records use float4 alignment. Leaf ranges index reordered triangles.
    struct PackedNode {
      float lo[3];
      uint32_t begin;
      float hi[3];
      uint32_t count;
      uint32_t escape, padding[3];
    };
    struct PackedTriangle {
      float a[4], b[4], c[4], albedo[4], emission[4];
    };
    static_assert(sizeof(PackedNode) == 48 && sizeof(PackedTriangle) == 80);
    const uint32_t triangleOffset =
        16 + uint32_t(nodes.size()) * sizeof(PackedNode);
    std::vector<uint32_t> result(
        (triangleOffset + order.size() * sizeof(PackedTriangle)) / 4);
    result[0] = uint32_t(nodes.size());
    result[1] = triangleOffset;
    std::vector<uint32_t> escapes(nodes.size());
    for (size_t i = nodes.size(); i-- > 0;) {
      const auto &n = nodes[i];
      escapes[i] = n.left < 0 ? uint32_t(i + 1) : escapes[n.right];
      const PackedNode packed{{n.lo.x, n.lo.y, n.lo.z},
                              uint32_t(n.begin),
                              {n.hi.x, n.hi.y, n.hi.z},
                              n.left < 0 ? uint32_t(n.end - n.begin) : 0,
                              escapes[i],
                              {0, 0, 0}};
      std::memcpy(reinterpret_cast<char *>(result.data()) + 16 +
                      i * sizeof(PackedNode),
                  &packed, sizeof(packed));
    }
    for (size_t i = 0; i < order.size(); ++i) {
      const auto &t = triangles[order[i]];
      const PackedTriangle packed{
          {t.a.x, t.a.y, t.a.z, 0},
          {t.b.x, t.b.y, t.b.z, 0},
          {t.c.x, t.c.y, t.c.z, 0},
          {t.albedo.x, t.albedo.y, t.albedo.z, 0},
          {t.emission.x, t.emission.y, t.emission.z, 0}};
      std::memcpy(reinterpret_cast<char *>(result.data()) + triangleOffset +
                      i * sizeof(PackedTriangle),
                  &packed, sizeof(packed));
    }
    return result;
  }
  const Triangle *intersect(const Vec3 &origin, const Vec3 &direction,
                            float &distance) const {
    const Triangle *hit = nullptr;
    if (!nodes.empty())
      intersectNode(0, origin, direction, distance, hit);
    return hit;
  }
  Vec3 direct(const Light &light, const Vec3 &position,
              const Vec3 &normal) const {
    return std::visit(
        [&](const auto &source) {
          using T = std::decay_t<decltype(source)>;
          Vec3 direction;
          float distance = std::numeric_limits<float>::infinity();
          float strength = source.getIntensity();
          if constexpr (std::is_same_v<T, DirectionalLight>) {
            direction = -1.0f * source.getDirection();
          } else {
            direction = source.getPosition() - position;
            const float squared = std::max(direction.lengthSqrt(), 1e-6f);
            distance = std::sqrt(squared);
            direction /= distance;
            strength /=
                squared * 0.23f; // Match the PBR renderer's light units.
            if constexpr (std::is_same_v<T, SpotLight>) {
              const float inner =
                  std::cos(source.getInnerAngle() * pi / 180.0f);
              const float outer =
                  std::cos(source.getOuterAngle() * pi / 180.0f);
              const float angle =
                  Vec3::dot(-1.0f * direction, source.getDirection());
              const float t =
                  inner > outer ? std::clamp((angle - outer) / (inner - outer),
                                             0.0f, 1.0f)
                                : (angle >= outer ? 1.0f : 0.0f);
              strength *= t * t * (3.0f - 2.0f * t);
            }
          }
          const float cosine = std::max(Vec3::dot(normal, direction), 0.0f);
          if (cosine == 0.0f || strength == 0.0f)
            return Vec3(0);
          distance -= epsilon * 4.0f;
          if (intersect(position + normal * (epsilon * 2.0f), direction,
                        distance))
            return Vec3(0);
          return rgb(source.getColor()) * (strength * cosine / pi);
        },
        light);
  }
  Vec3 trace(Vec3 origin, Vec3 direction, const Vec3 &sky,
             const std::optional<Light> &light, UInt32 bounces,
             uint32_t &state) const {
    Vec3 result, throughput(1.0f);
    for (UInt32 bounce = 0; bounce < bounces; ++bounce) {
      float distance = std::numeric_limits<float>::infinity();
      const auto *hit = intersect(origin, direction, distance);
      if (!hit) {
        result += throughput * sky;
        break;
      }
      const Vec3 position = origin + direction * distance;
      Vec3 normal =
          Vec3::normalize(Vec3::cross(hit->b - hit->a, hit->c - hit->a));
      if (Vec3::dot(normal, direction) > 0.0f)
        normal *= -1.0f;
      result += throughput * hit->emission;
      if (light)
        result += throughput * hit->albedo * direct(*light, position, normal);
      throughput = throughput * hit->albedo;
      origin = position + normal * (epsilon * 2.0f);
      direction = cosineDirection(normal, state);
    }
    return result;
  }
};
} // namespace

struct GlobalIllumination::RealtimeState {
  std::weak_ptr<gpu::Device> device;
  Ptr<gpu::ComputePipeline> pipeline;
  Ptr<gpu::Buffer> scene, probes;
  UInt32 bounces = 2, frame = 0, bytesPerRow = 0;
  bool geometryDirty = true;
};

void GlobalIllumination::invalidate() {
  if (realtime) {
    realtime->geometryDirty = true;
    return;
  }
  irradiance.clear();
  texture.reset();
}
void GlobalIllumination::clear() {
  realtime.reset();
  triangles.clear();
  light.reset();
  sky = Vec3(0);
  invalidate();
}
void GlobalIllumination::beginFrame() {
  triangles.clear();
  invalidate();
}

bool GlobalIllumination::initializeRealtime(const Vec3 &lo, const Vec3 &hi,
                                            UInt32 nx, UInt32 ny, UInt32 nz,
                                            UInt32 bounces) {
  const auto device = Graphics::getDevice();
  const Vec3 extent = hi - lo;
  if (!device || device->getBackendAPI() == GPUBackendAPI::SDLGPU ||
      !finite(lo) || !finite(hi) || !finite(extent) || extent.x <= epsilon ||
      extent.y <= epsilon || extent.z <= epsilon || nx < 2 || ny < 2 ||
      nz < 2 || nx > 32 || ny > 32 || nz > 32 || bounces < 1 || bounces > 4)
    return false;
  auto shader = BuiltinShader::createFromBundle(
      renderer_gi_comp_snb, renderer_gi_comp_snb_len, ShaderStage::Compute);
  if (!shader.getRaw())
    return false;
  auto state = std::make_shared<RealtimeState>();
  state->device = device;
  state->pipeline = device->createComputePipeline(
      {GlobalAllocator::get(), shader.getRaw(), 64, 1, 1});
  if (!state->pipeline)
    return false;
  state->bounces = bounces;
  state->bytesPerRow = (nx * 6 * 16 + 255) & ~255u;
  state->probes =
      device->createBuffer({GlobalAllocator::get(), gpu::BufferUsage::Storage,
                            state->bytesPerRow * ny * nz});
  if (!state->probes)
    return false;
  std::vector<float> zeros(size_t(nx) * ny * nz * 6 * 4, 0.0f);
  auto atlas = Texture::create();
  atlas->loadFromMemory(zeros.data(), nx * 6, ny * nz,
                        gpu::TextureFormat::R32G32B32A32_FLOAT, 16);
  if (!atlas->getRaw())
    return false;
  minimum = lo;
  maximum = hi;
  countX = nx;
  countY = ny;
  countZ = nz;
  irradiance.clear();
  texture = std::move(atlas);
  realtime = std::move(state);
  return true;
}

bool GlobalIllumination::update(float historyWeight) {
  const auto device = Graphics::getDevice();
  if (!realtime || !device || device != realtime->device.lock() ||
      !std::isfinite(historyWeight))
    return false;
  auto &state = *realtime;
  auto commands = device->acquireCommandBuffer({GlobalAllocator::get()});
  if (!commands)
    return false;
  if (state.geometryDirty) {
    const auto packed = RayScene(triangles).pack();
    const auto size = static_cast<UInt32>(packed.size() * sizeof(uint32_t));
    auto scene = device->createBuffer(
        {GlobalAllocator::get(), gpu::BufferUsage::Storage, size});
    auto upload = device->createTransferBuffer(
        {GlobalAllocator::get(), gpu::TransferBufferUsage::Upload, size});
    if (!scene || !upload)
      return false;
    auto mapped = upload->map(false);
    if (!mapped)
      return false;
    std::memcpy(mapped, packed.data(), size);
    upload->unmap();
    auto copy = commands->beginCopyPass();
    copy->uploadBuffer({upload, 0}, {scene, 0, size}, false);
    commands->endCopyPass(copy);
    state.scene = scene;
    state.geometryDirty = false;
  }
  struct Uniform {
    float minimum[4], extent[4];
    uint32_t counts[4], settings[4];
    float skyAndHistory[4];
    float lightPositionAndType[4], lightDirectionAndIntensity[4],
        lightColorAndInner[4], lightOuter[4];
  } uniforms{};
  static_assert(sizeof(Uniform) == 144);
  const auto extent = maximum - minimum;
  uniforms.minimum[0] = minimum.x;
  uniforms.minimum[1] = minimum.y;
  uniforms.minimum[2] = minimum.z;
  uniforms.extent[0] = extent.x;
  uniforms.extent[1] = extent.y;
  uniforms.extent[2] = extent.z;
  uniforms.counts[0] = countX;
  uniforms.counts[1] = countY;
  uniforms.counts[2] = countZ;
  uniforms.settings[0] = state.frame;
  uniforms.settings[1] = state.bounces;
  uniforms.settings[2] = state.bytesPerRow;
  uniforms.skyAndHistory[0] = sky.x;
  uniforms.skyAndHistory[1] = sky.y;
  uniforms.skyAndHistory[2] = sky.z;
  uniforms.skyAndHistory[3] =
      state.frame ? std::clamp(historyWeight, 0.0f, 0.99f) : 0.0f;
  if (light)
    std::visit(
        [&](const auto &source) {
          using T = std::decay_t<decltype(source)>;
          Vec3 p, d;
          float type = 0, inner = 1, outer = 1;
          if constexpr (!std::is_same_v<T, DirectionalLight>) {
            p = source.getPosition();
            type = 1;
          }
          if constexpr (!std::is_same_v<T, PointLight>)
            d = source.getDirection();
          if constexpr (std::is_same_v<T, SpotLight>) {
            type = 2;
            inner = std::cos(source.getInnerAngle() * pi / 180);
            outer = std::cos(source.getOuterAngle() * pi / 180);
          }
          const auto color = rgb(source.getColor());
          const float data[16] = {
              p.x,     p.y,     p.z,     type,
              d.x,     d.y,     d.z,     source.getIntensity(),
              color.x, color.y, color.z, inner,
              outer,   0,       0,       0};
          std::memcpy(uniforms.lightPositionAndType, data, sizeof(data));
        },
        *light);
  commands->pushComputeUniformData(0, &uniforms, sizeof(uniforms));
  Array<gpu::StorageTextureBinding> textures;
  Array<gpu::StorageBufferBinding> buffers;
  buffers.push_back({state.scene, false});
  buffers.push_back({state.probes, false});
  auto pass = commands->beginComputePass(textures, buffers);
  pass->bindComputePipeline(state.pipeline);
  pass->dispatchWorkgroups(countX, countY, countZ);
  commands->endComputePass(pass);
  auto copy = commands->beginCopyPass();
  gpu::TextureRegion region{};
  region.texture = texture->getRaw();
  region.width = countX * 6;
  region.height = countY * countZ;
  region.depth = 1;
  const bool copied = copy->copyBufferToTexture(
      {state.probes, 0, state.probes->getCreateInfo().size}, region,
      state.bytesPerRow);
  commands->endCopyPass(copy);
  device->submitCommandBuffer(commands);
  if (!copied)
    return false;
  ++state.frame;
  return true;
}
void GlobalIllumination::setLight(const Light &value) {
  light = value;
  if (!realtime)
    invalidate();
}
void GlobalIllumination::clearLight() {
  light.reset();
  if (!realtime)
    invalidate();
}
void GlobalIllumination::setSkyColor(const Color &value) {
  sky = rgb(value);
  if (!realtime)
    invalidate();
}

void GlobalIllumination::addModel(const Model &model,
                                  const Transform &transform,
                                  const Color &albedo, const Color &emission,
                                  UInt32 triangleBudget) {
  if (!model.getParts().empty()) {
    const auto total = model.getTriangleCount();
    for (const auto &part : model.getParts()) {
      const auto budget =
          triangleBudget && total
              ? std::max(1u,
                         static_cast<UInt32>(uint64_t(triangleBudget) *
                                             part->getTriangleCount() / total))
              : 0u;
      addModel(*part, transform, albedo, emission, budget);
    }
    return;
  }
  invalidate();
  const auto mesh = model.getMesh().data();
  if (!mesh || mesh->vertices.empty())
    return;
  const auto base = model.getTexture(TextureKey::BaseColor);
  const auto &surface = model.getSurfaceProperties();
  if (triangleBudget &&
      (surface.alphaCutoff > 0 ||
       (surface.autoAlphaMask && base && base->hasTransparency())))
    return;
  const auto reflectance =
      rgb(albedo) * (base ? rgb(base->averageColor()) : Vec3(1));
  auto radiance = rgb(emission);
  if (const auto image = model.getTexture(TextureKey::Emissive))
    radiance += rgb(image->averageColor(false)) * rgb(surface.emissiveFactor);
  Array<UInt32> simplified;
  const auto *indices = &mesh->indices;
  if (triangleBudget && triangleBudget < mesh->indices.size() / 3) {
    simplified.resize(mesh->indices.size());
    const auto count = meshopt_simplifySloppy(
        simplified.data(), mesh->indices.data(), mesh->indices.size(),
        &mesh->vertices[0].position.x, mesh->vertices.size(), sizeof(Vertex),
        nullptr, size_t(triangleBudget) * 3, 1.0f, nullptr);
    if (count) {
      simplified.resize(count);
      indices = &simplified;
    }
  }
  // World matrices use column vectors in the renderer; Vec3::transform uses
  // row vectors, so transpose here to match the vertex shader exactly.
  const auto matrix = Mat4::transpose(transform.getWorldMatrix());
  for (size_t i = 0; i + 2 < indices->size(); i += 3) {
    const auto ia = (*indices)[i], ib = (*indices)[i + 1],
               ic = (*indices)[i + 2];
    if (ia >= mesh->vertices.size() || ib >= mesh->vertices.size() ||
        ic >= mesh->vertices.size())
      continue;
    const auto &a = mesh->vertices[ia], &b = mesh->vertices[ib],
               &c = mesh->vertices[ic];
    Triangle t{Vec3::transform(a.position, matrix),
               Vec3::transform(b.position, matrix),
               Vec3::transform(c.position, matrix),
               reflectance * ((rgb(a.color) + rgb(b.color) + rgb(c.color)) *
                              (1.0f / 3.0f)),
               radiance};
    if (!finite(t.a) || !finite(t.b) || !finite(t.c) ||
        Vec3::cross(t.b - t.a, t.c - t.a).lengthSqrt() < 1e-16f)
      continue;
    t.albedo = Vec3(std::min(t.albedo.x, 1.0f), std::min(t.albedo.y, 1.0f),
                    std::min(t.albedo.z, 1.0f));
    triangles.push_back(t);
  }
}

bool GlobalIllumination::bake(const Vec3 &lo, const Vec3 &hi, UInt32 nx,
                              UInt32 ny, UInt32 nz, UInt32 rays,
                              UInt32 bounces) {
  const Vec3 extent = hi - lo;
  if (!finite(lo) || !finite(hi) || !finite(extent) || extent.x <= epsilon ||
      extent.y <= epsilon || extent.z <= epsilon || nx < 2 || ny < 2 ||
      nz < 2 || nx > 32 || ny > 32 || nz > 32 || rays < 32 || rays > 65536 ||
      bounces < 1 || bounces > 8)
    return false;
  const RayScene scene(triangles);
  std::vector<Vec3> baked(static_cast<size_t>(nx) * ny * nz * 6);
  for (UInt32 z = 0; z < nz; ++z)
    for (UInt32 y = 0; y < ny; ++y)
      for (UInt32 x = 0; x < nx; ++x) {
        const size_t probe = (static_cast<size_t>(z) * ny + y) * nx + x;
        const Vec3 position =
            lo + extent * Vec3(float(x) / (nx - 1), float(y) / (ny - 1),
                               float(z) / (nz - 1));
        uint32_t state = 0x9e3779b9u ^ static_cast<uint32_t>(probe + 1);
        for (UInt32 ray = 0; ray < rays; ++ray) {
          // Stratified sphere directions; subsequent diffuse hits use cosine
          // sampling.
          const float h = 1.0f - 2.0f * (float(ray) + 0.5f) / rays;
          const float phi = 2.0f * pi * random01(state);
          const float r = std::sqrt(std::max(0.0f, 1.0f - h * h));
          const Vec3 direction(r * std::cos(phi), h, r * std::sin(phi));
          const Vec3 radiance =
              scene.trace(position, direction, sky, light, bounces, state);
          for (size_t axis = 0; axis < axes.size(); ++axis)
            baked[probe * 6 + axis] +=
                radiance * (4.0f * pi / rays *
                            std::max(Vec3::dot(axes[axis], direction), 0.0f));
        }
      }
  minimum = lo;
  realtime.reset();
  maximum = hi;
  countX = nx;
  countY = ny;
  countZ = nz;
  irradiance = std::move(baked);
  texture.reset();
  return true;
}

Vec3 GlobalIllumination::getProbeCounts() const {
  return Vec3(float(countX), float(countY), float(countZ));
}
Vec3 GlobalIllumination::sampleIrradiance(const Vec3 &position,
                                          const Vec3 &normal) const {
  if (!isBaked() || !finite(position) || !finite(normal) ||
      normal.lengthSqrt() < 1e-12f)
    return Vec3(0);
  const Vec3 n = Vec3::normalize(normal);
  const Vec3 p =
      (position - minimum) / (maximum - minimum) * (getProbeCounts() - Vec3(1));
  const Vec3 cell(std::clamp(p.x, 0.0f, float(countX - 1)),
                  std::clamp(p.y, 0.0f, float(countY - 1)),
                  std::clamp(p.z, 0.0f, float(countZ - 1)));
  const UInt32 ix = std::min(UInt32(cell.x), countX - 2),
               iy = std::min(UInt32(cell.y), countY - 2),
               iz = std::min(UInt32(cell.z), countZ - 2);
  const Vec3 f = cell - Vec3(float(ix), float(iy), float(iz));
  Vec3 result;
  for (UInt32 z = 0; z < 2; ++z)
    for (UInt32 y = 0; y < 2; ++y)
      for (UInt32 x = 0; x < 2; ++x) {
        const size_t probe =
            ((static_cast<size_t>(iz + z) * countY + iy + y) * countX + ix +
             x) *
            6;
        const float weight =
            (x ? f.x : 1 - f.x) * (y ? f.y : 1 - f.y) * (z ? f.z : 1 - f.z);
        result += (irradiance[probe + (n.x >= 0 ? 0 : 1)] * (n.x * n.x) +
                   irradiance[probe + (n.y >= 0 ? 2 : 3)] * (n.y * n.y) +
                   irradiance[probe + (n.z >= 0 ? 4 : 5)] * (n.z * n.z)) *
                  weight;
      }
  return result;
}
Ptr<Texture> GlobalIllumination::getTexture() const {
  if (realtime)
    return texture;
  if (!isBaked())
    return {};
  if (!texture) {
    std::vector<float> pixels(irradiance.size() * 4);
    for (size_t i = 0; i < irradiance.size(); ++i) {
      pixels[i * 4] = irradiance[i].x;
      pixels[i * 4 + 1] = irradiance[i].y;
      pixels[i * 4 + 2] = irradiance[i].z;
      pixels[i * 4 + 3] = 1.0f;
    }
    auto uploaded = Texture::create();
    uploaded->loadFromMemory(pixels.data(), countX * 6, countY * countZ,
                             gpu::TextureFormat::R32G32B32A32_FLOAT, 16);
    if (uploaded->getRaw())
      texture = std::move(uploaded);
  }
  return texture;
}
} // namespace sinen
