#include <physics/physics.hpp>

#include <algorithm>

#include <box2d/box2d.h>
#include <box3d/box3d.h>

#include <core/data/hashmap.hpp>
#include <core/data/ptr.hpp>
#include <core/def/types.hpp>
#include <math/math.hpp>
#include <math/quaternion.hpp>
#include <math/transform/transform.hpp>
#include <math/vec3.hpp>
#include <physics/world2d.hpp>
#include <physics/world3d.hpp>

namespace sinen {
namespace {
static UInt32 nextColliderID = 1;

UInt32 getNextId() { return nextColliderID++; }

b3Vec3 toB3(const Vec3 &v) { return {v.x, v.y, v.z}; }

Vec3 fromB3(const b3Vec3 &v) { return {v.x, v.y, v.z}; }

b3Quat toB3(const Quat &q) { return {{q.x, q.y, q.z}, q.w}; }

Quat fromB3(const b3Quat &q) { return Quat(q.v.x, q.v.y, q.v.z, q.s); }

b2Rot rotation2D(float degrees) { return b2MakeRot(Math::toRadians(-degrees)); }

float rotationFromB2(const b2Rot &rotation) {
  return -Math::toDegrees(b2Rot_GetAngle(rotation));
}

Vec3 rotation3D(const b3Quat &q) {
  const Vec3 euler = Quat::toEuler(fromB3(q));
  return {Math::toDegrees(euler.x), Math::toDegrees(euler.y),
          Math::toDegrees(euler.z)};
}

b3Quat rotationFromEuler(const Vec3 &rotation) {
  return toB3(Quat::fromEuler(rotation));
}

b3BodyType bodyType3D(bool isStatic) {
  return isStatic ? b3_staticBody : b3_dynamicBody;
}

b2BodyType bodyType2D(bool isStatic) {
  return isStatic ? b2_staticBody : b2_dynamicBody;
}

b3ShapeDef shapeDef3D() {
  b3ShapeDef def = b3DefaultShapeDef();
  def.density = 1.0f;
  return def;
}

b2ShapeDef shapeDef2D() {
  b2ShapeDef def = b2DefaultShapeDef();
  def.density = 1.0f;
  return def;
}

struct Body3D {
  b3BodyId body;
  b3ShapeId shape;
};

struct Body2D {
  b2BodyId body;
  b2ShapeId shape;
};

class World3DImpl : public World3D {
public:
  World3DImpl() {
    b3WorldDef def = b3DefaultWorldDef();
    def.gravity = {0.0f, -9.8f, 0.0f};
    world = b3CreateWorld(&def);
  }

  ~World3DImpl() override {
    if (b3World_IsValid(world)) {
      b3DestroyWorld(world);
    }
  }

  bool isValid(const Collider &collider) override {
    auto entry = bodyMap.find(collider.id);
    return entry != bodyMap.end() && b3Body_IsValid(entry->second.body);
  }

  bool isAdded(const Collider &collider) override {
    Body3D *entry = findBody(collider);
    return entry != nullptr && b3Body_IsEnabled(entry->body);
  }

  Vec3 getPosition(const Collider &collider) override {
    Body3D *entry = findBody(collider);
    if (entry == nullptr) {
      return Vec3(0.0f);
    }
    return fromB3(b3Body_GetPosition(entry->body));
  }

  Vec3 getRotation(const Collider &collider) override {
    Body3D *entry = findBody(collider);
    if (entry == nullptr) {
      return Vec3(0.0f);
    }
    return rotation3D(b3Body_GetRotation(entry->body));
  }

  Vec3 getVelocity(const Collider &collider) override {
    Body3D *entry = findBody(collider);
    if (entry == nullptr) {
      return Vec3(0.0f);
    }
    return fromB3(b3Body_GetLinearVelocity(entry->body));
  }

  Vec3 getAngularVelocity(const Collider &collider) override {
    Body3D *entry = findBody(collider);
    if (entry == nullptr) {
      return Vec3(0.0f);
    }
    return fromB3(b3Body_GetAngularVelocity(entry->body));
  }

  void setPosition(const Collider &collider, const Vec3 &position,
                   bool activate) override {
    Body3D *entry = findBody(collider);
    if (entry == nullptr) {
      return;
    }
    b3Body_SetTransform(entry->body, toB3(position),
                        b3Body_GetRotation(entry->body));
    wake(entry->body, activate);
  }

  void setRotation(const Collider &collider, const Vec3 &rotation,
                   bool activate) override {
    Body3D *entry = findBody(collider);
    if (entry == nullptr) {
      return;
    }
    b3Body_SetTransform(entry->body, b3Body_GetPosition(entry->body),
                        rotationFromEuler(rotation));
    wake(entry->body, activate);
  }

  void setPositionAndRotation(const Collider &collider, const Vec3 &position,
                              const Vec3 &rotation, bool activate) override {
    Body3D *entry = findBody(collider);
    if (entry == nullptr) {
      return;
    }
    b3Body_SetTransform(entry->body, toB3(position),
                        rotationFromEuler(rotation));
    wake(entry->body, activate);
  }

  void setLinearVelocity(const Collider &collider,
                         const Vec3 &velocity) override {
    Body3D *entry = findBody(collider);
    if (entry != nullptr) {
      b3Body_SetLinearVelocity(entry->body, toB3(velocity));
    }
  }

  void setAngularVelocity(const Collider &collider,
                          const Vec3 &velocity) override {
    Body3D *entry = findBody(collider);
    if (entry != nullptr) {
      b3Body_SetAngularVelocity(entry->body, toB3(velocity));
    }
  }

  void addForce(const Collider &collider, const Vec3 &force,
                bool activate) override {
    Body3D *entry = findBody(collider);
    if (entry != nullptr) {
      b3Body_ApplyForceToCenter(entry->body, toB3(force), activate);
    }
  }

  void addImpulse(const Collider &collider, const Vec3 &impulse) override {
    Body3D *entry = findBody(collider);
    if (entry != nullptr) {
      b3Body_ApplyLinearImpulseToCenter(entry->body, toB3(impulse), true);
    }
  }

  void setFriction(const Collider &collider, float friction) override {
    Body3D *entry = findBody(collider);
    if (entry != nullptr) {
      b3Shape_SetFriction(entry->shape, friction);
    }
  }

  void setRestitution(const Collider &collider, float restitution) override {
    Body3D *entry = findBody(collider);
    if (entry != nullptr) {
      b3Shape_SetRestitution(entry->shape, restitution);
    }
  }

  void activate(const Collider &collider) override {
    Body3D *entry = findBody(collider);
    if (entry != nullptr) {
      b3Body_SetAwake(entry->body, true);
    }
  }

  void deactivate(const Collider &collider) override {
    Body3D *entry = findBody(collider);
    if (entry != nullptr) {
      b3Body_SetAwake(entry->body, false);
    }
  }

  void setGravity(const Vec3 &gravity) override {
    b3World_SetGravity(world, toB3(gravity));
  }

  Vec3 getGravity() override { return fromB3(b3World_GetGravity(world)); }

  UInt32 bodyCount() override {
    return static_cast<UInt32>(b3World_GetCounters(world).bodyCount);
  }

  Collider newBoxCollider(const Transform &transform, bool isStatic) override {
    b3BodyId body =
        createBody(transform.position, transform.rotation, isStatic);
    b3ShapeDef def = shapeDef3D();
    b3BoxHull box = b3MakeBoxHull(std::max(transform.scale.x, 0.001f),
                                  std::max(transform.scale.y, 0.001f),
                                  std::max(transform.scale.z, 0.001f));
    return store(body, b3CreateHullShape(body, &def, &box.base));
  }

  Collider newSphereCollider(const Vec3 &position, float radius,
                             bool isStatic) override {
    b3BodyId body = createBody(position, Vec3(0.0f), isStatic);
    b3ShapeDef def = shapeDef3D();
    b3Sphere sphere = {b3Vec3_zero, std::max(radius, 0.001f)};
    return store(body, b3CreateSphereShape(body, &def, &sphere));
  }

  Collider newCylinderCollider(const Vec3 &position, const Vec3 &rotation,
                               float halfHeight, float radius,
                               bool isStatic) override {
    b3BodyId body = createBody(position, rotation, isStatic);
    b3ShapeDef def = shapeDef3D();
    b3HullData *cylinder = b3CreateCylinder(std::max(halfHeight * 2.0f, 0.001f),
                                            std::max(radius, 0.001f), 0.0f, 24);
    b3ShapeId shape = b3CreateHullShape(body, &def, cylinder);
    b3DestroyHull(cylinder);
    return store(body, shape);
  }

  Collider newCapsuleCollider(const Vec3 &position, const Vec3 &rotation,
                              float halfHeight, float radius,
                              bool isStatic) override {
    b3BodyId body = createBody(position, rotation, isStatic);
    b3ShapeDef def = shapeDef3D();
    b3Capsule capsule = {{0.0f, -halfHeight, 0.0f},
                         {0.0f, halfHeight, 0.0f},
                         std::max(radius, 0.001f)};
    return store(body, b3CreateCapsuleShape(body, &def, &capsule));
  }

  void addCollider(const Collider &collider, bool active) override {
    Body3D *entry = findBody(collider);
    if (entry != nullptr) {
      b3Body_Enable(entry->body);
      b3Body_SetAwake(entry->body, active);
    }
  }

  void removeCollider(const Collider &collider) override {
    Body3D *entry = findBody(collider);
    if (entry != nullptr && b3Body_IsEnabled(entry->body)) {
      b3Body_Disable(entry->body);
    }
  }

  void destroyCollider(const Collider &collider) override {
    auto entry = bodyMap.find(collider.id);
    if (entry == bodyMap.end()) {
      return;
    }
    if (b3Body_IsValid(entry->second.body)) {
      b3DestroyBody(entry->second.body);
    }
    bodyMap.erase(entry);
  }

  void optimizeBroadPhase() override {}

  void update(float time, int collisionSteps) override {
    b3World_Step(world, time, std::max(collisionSteps, 1));
  }

private:
  Hashmap<UInt32, Body3D> bodyMap = {};
  b3WorldId world = {};

  Body3D *findBody(const Collider &collider) {
    auto entry = bodyMap.find(collider.id);
    if (entry == bodyMap.end() || !b3Body_IsValid(entry->second.body)) {
      return nullptr;
    }
    return &entry->second;
  }

  b3BodyId createBody(const Vec3 &position, const Vec3 &rotation,
                      bool isStatic) {
    b3BodyDef def = b3DefaultBodyDef();
    def.type = bodyType3D(isStatic);
    def.position = toB3(position);
    def.rotation = rotationFromEuler(rotation);
    def.isEnabled = false;
    return b3CreateBody(world, &def);
  }

  Collider store(b3BodyId body, b3ShapeId shape) {
    Collider collider{*this, getNextId()};
    bodyMap[collider.id] = {body, shape};
    return collider;
  }

  static void wake(b3BodyId body, bool activate) {
    if (activate) {
      b3Body_SetAwake(body, true);
    }
  }
};

class World2DImpl : public World2D {
public:
  World2DImpl() {
    b2WorldDef def = b2DefaultWorldDef();
    def.gravity = {0.0f, -9.8f};
    world = b2CreateWorld(&def);
  }

  ~World2DImpl() override {
    if (b2World_IsValid(world)) {
      b2DestroyWorld(world);
    }
  }

  bool isValid(const Collider &collider) override {
    auto entry = bodyMap.find(collider.id);
    return entry != bodyMap.end() && b2Body_IsValid(entry->second.body);
  }

  bool isAdded(const Collider &collider) override {
    Body2D *entry = findBody(collider);
    return entry != nullptr && b2Body_IsEnabled(entry->body);
  }

  Vec3 getPosition(const Collider &collider) override {
    Body2D *entry = findBody(collider);
    if (entry == nullptr) {
      return Vec3(0.0f);
    }
    b2Pos p = b2Body_GetPosition(entry->body);
    return {p.x, p.y, 0.0f};
  }

  Vec3 getRotation(const Collider &collider) override {
    Body2D *entry = findBody(collider);
    if (entry == nullptr) {
      return Vec3(0.0f);
    }
    return {0.0f, 0.0f, rotationFromB2(b2Body_GetRotation(entry->body))};
  }

  Vec3 getVelocity(const Collider &collider) override {
    Body2D *entry = findBody(collider);
    if (entry == nullptr) {
      return Vec3(0.0f);
    }
    b2Vec2 velocity = b2Body_GetLinearVelocity(entry->body);
    return {velocity.x, velocity.y, 0.0f};
  }

  Vec3 getAngularVelocity(const Collider &collider) override {
    Body2D *entry = findBody(collider);
    if (entry == nullptr) {
      return Vec3(0.0f);
    }
    return {0.0f, 0.0f, -b2Body_GetAngularVelocity(entry->body)};
  }

  void setPosition(const Collider &collider, const Vec3 &position,
                   bool activate) override {
    Body2D *entry = findBody(collider);
    if (entry == nullptr) {
      return;
    }
    b2Body_SetTransform(entry->body, {position.x, position.y},
                        b2Body_GetRotation(entry->body));
    wake(entry->body, activate);
  }

  void setRotation(const Collider &collider, const Vec3 &rotation,
                   bool activate) override {
    Body2D *entry = findBody(collider);
    if (entry == nullptr) {
      return;
    }
    b2Body_SetTransform(entry->body, b2Body_GetPosition(entry->body),
                        rotation2D(rotation.z));
    wake(entry->body, activate);
  }

  void setPositionAndRotation(const Collider &collider, const Vec3 &position,
                              const Vec3 &rotation, bool activate) override {
    Body2D *entry = findBody(collider);
    if (entry == nullptr) {
      return;
    }
    b2Body_SetTransform(entry->body, {position.x, position.y},
                        rotation2D(rotation.z));
    wake(entry->body, activate);
  }

  void setLinearVelocity(const Collider &collider,
                         const Vec3 &velocity) override {
    Body2D *entry = findBody(collider);
    if (entry != nullptr) {
      b2Body_SetLinearVelocity(entry->body, {velocity.x, velocity.y});
    }
  }

  void setAngularVelocity(const Collider &collider,
                          const Vec3 &velocity) override {
    Body2D *entry = findBody(collider);
    if (entry != nullptr) {
      b2Body_SetAngularVelocity(entry->body, -velocity.z);
    }
  }

  void addForce(const Collider &collider, const Vec3 &force,
                bool activate) override {
    Body2D *entry = findBody(collider);
    if (entry != nullptr) {
      b2Body_ApplyForceToCenter(entry->body, {force.x, force.y}, activate);
    }
  }

  void addImpulse(const Collider &collider, const Vec3 &impulse) override {
    Body2D *entry = findBody(collider);
    if (entry != nullptr) {
      b2Body_ApplyLinearImpulseToCenter(entry->body, {impulse.x, impulse.y},
                                        true);
    }
  }

  void setFriction(const Collider &collider, float friction) override {
    Body2D *entry = findBody(collider);
    if (entry != nullptr) {
      b2Shape_SetFriction(entry->shape, friction);
    }
  }

  void setRestitution(const Collider &collider, float restitution) override {
    Body2D *entry = findBody(collider);
    if (entry != nullptr) {
      b2Shape_SetRestitution(entry->shape, restitution);
    }
  }

  void activate(const Collider &collider) override {
    Body2D *entry = findBody(collider);
    if (entry != nullptr) {
      b2Body_SetAwake(entry->body, true);
    }
  }

  void deactivate(const Collider &collider) override {
    Body2D *entry = findBody(collider);
    if (entry != nullptr) {
      b2Body_SetAwake(entry->body, false);
    }
  }

  void setGravity(const Vec3 &gravity) override {
    b2World_SetGravity(world, {gravity.x, gravity.y});
  }

  Vec3 getGravity() override {
    b2Vec2 gravity = b2World_GetGravity(world);
    return {gravity.x, gravity.y, 0.0f};
  }

  UInt32 bodyCount() override {
    return static_cast<UInt32>(b2World_GetCounters(world).bodyCount);
  }

  Collider newBoxCollider(const Transform &transform, bool isStatic) override {
    b2BodyId body =
        createBody(transform.position, transform.rotation.z, isStatic);
    b2ShapeDef def = shapeDef2D();
    b2Polygon box = b2MakeBox(std::max(transform.scale.x, 0.001f),
                              std::max(transform.scale.y, 0.001f));
    return store(body, b2CreatePolygonShape(body, &def, &box));
  }

  Collider newCircleCollider(const Vec3 &position, float radius,
                             bool isStatic) override {
    b2BodyId body = createBody(position, 0.0f, isStatic);
    b2ShapeDef def = shapeDef2D();
    b2Circle circle = {{0.0f, 0.0f}, std::max(radius, 0.001f)};
    return store(body, b2CreateCircleShape(body, &def, &circle));
  }

  Collider newCapsuleCollider(const Vec3 &position, const Vec3 &rotation,
                              float halfHeight, float radius,
                              bool isStatic) override {
    b2BodyId body = createBody(position, rotation.z, isStatic);
    b2ShapeDef def = shapeDef2D();
    b2Capsule capsule = {
        {0.0f, -halfHeight}, {0.0f, halfHeight}, std::max(radius, 0.001f)};
    return store(body, b2CreateCapsuleShape(body, &def, &capsule));
  }

  void addCollider(const Collider &collider, bool active) override {
    Body2D *entry = findBody(collider);
    if (entry != nullptr) {
      b2Body_Enable(entry->body);
      b2Body_SetAwake(entry->body, active);
    }
  }

  void removeCollider(const Collider &collider) override {
    Body2D *entry = findBody(collider);
    if (entry != nullptr && b2Body_IsEnabled(entry->body)) {
      b2Body_Disable(entry->body);
    }
  }

  void destroyCollider(const Collider &collider) override {
    auto entry = bodyMap.find(collider.id);
    if (entry == bodyMap.end()) {
      return;
    }
    if (b2Body_IsValid(entry->second.body)) {
      b2DestroyBody(entry->second.body);
    }
    bodyMap.erase(entry);
  }

  void optimizeBroadPhase() override {}

  void update(float time, int collisionSteps) override {
    b2World_Step(world, time, std::max(collisionSteps, 1));
  }

private:
  Hashmap<UInt32, Body2D> bodyMap = {};
  b2WorldId world = {};

  Body2D *findBody(const Collider &collider) {
    auto entry = bodyMap.find(collider.id);
    if (entry == bodyMap.end() || !b2Body_IsValid(entry->second.body)) {
      return nullptr;
    }
    return &entry->second;
  }

  b2BodyId createBody(const Vec3 &position, float rotationZ, bool isStatic) {
    b2BodyDef def = b2DefaultBodyDef();
    def.type = bodyType2D(isStatic);
    def.position = {position.x, position.y};
    def.rotation = rotation2D(rotationZ);
    def.isEnabled = false;
    return b2CreateBody(world, &def);
  }

  Collider store(b2BodyId body, b2ShapeId shape) {
    Collider collider{*this, getNextId()};
    bodyMap[collider.id] = {body, shape};
    return collider;
  }

  static void wake(b2BodyId body, bool activate) {
    if (activate) {
      b2Body_SetAwake(body, true);
    }
  }
};
} // namespace

bool Physics::initialize() { return true; }

void Physics::shutdown() {}

Ptr<World3D> World3D::create() { return makePtr<World3DImpl>(); }

Ptr<World2D> World2D::create() { return makePtr<World2DImpl>(); }

Collider::Collider(PhysicsWorld &world, UInt32 id) : world(world), id(id) {}
bool Collider::isValid() const { return world.isValid(*this); }
bool Collider::isAdded() const { return world.isAdded(*this); }
Vec3 Collider::getPosition() const { return world.getPosition(*this); }
Vec3 Collider::getRotation() const { return world.getRotation(*this); }
Vec3 Collider::getVelocity() const { return world.getVelocity(*this); }
Vec3 Collider::getAngularVelocity() const {
  return world.getAngularVelocity(*this);
}
void Collider::setPosition(const Vec3 &position, bool activate) const {
  world.setPosition(*this, position, activate);
}
void Collider::setRotation(const Vec3 &rotation, bool activate) const {
  world.setRotation(*this, rotation, activate);
}
void Collider::setPositionAndRotation(const Vec3 &position,
                                      const Vec3 &rotation,
                                      bool activate) const {
  world.setPositionAndRotation(*this, position, rotation, activate);
}
void Collider::setLinearVelocity(const Vec3 &velocity) const {
  world.setLinearVelocity(*this, velocity);
}
void Collider::setAngularVelocity(const Vec3 &velocity) const {
  world.setAngularVelocity(*this, velocity);
}
void Collider::addForce(const Vec3 &force, bool activate) const {
  world.addForce(*this, force, activate);
}
void Collider::addImpulse(const Vec3 &impulse) const {
  world.addImpulse(*this, impulse);
}
void Collider::setFriction(float friction) const {
  world.setFriction(*this, friction);
}
void Collider::setRestitution(float restitution) const {
  world.setRestitution(*this, restitution);
}
void Collider::activate() const { world.activate(*this); }
void Collider::deactivate() const { world.deactivate(*this); }
void Collider::remove() const { world.removeCollider(*this); }
void Collider::destroy() const { world.destroyCollider(*this); }
} // namespace sinen
