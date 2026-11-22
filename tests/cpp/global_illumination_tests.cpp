#include <graphics/model/mesh_builder.hpp>
#include <gtest/gtest.h>
#include <limits>
#include <renderer/global_illumination.hpp>

using namespace sinen;
namespace {
Model box(const Color &color) {
  MeshBuilder builder;
  builder.addBox(Vec3(4), color);
  Model model;
  *model.getMesh().data() = *builder.toMesh().data();
  return model;
}
} // namespace

TEST(GlobalIllumination, ConstantSkyIntegratesToPiAndInterpolatesAtBounds) {
  GlobalIllumination gi;
  gi.setSkyColor(Color(0.5f, 1.0f, 2.0f, 1.0f));
  ASSERT_TRUE(gi.bake(Vec3(-1), Vec3(1), 3, 2, 4, 4096, 2));
  for (const auto &p : {Vec3(-1), Vec3(0), Vec3(1), Vec3(100)}) {
    const auto e = gi.sampleIrradiance(p, Vec3(1, 2, -3));
    EXPECT_NEAR(e.x, 3.14159265f * 0.5f, 0.06f);
    EXPECT_NEAR(e.y, 3.14159265f, 0.12f);
    EXPECT_NEAR(e.z, 3.14159265f * 2.0f, 0.24f);
  }
  EXPECT_EQ(gi.sampleIrradiance(Vec3(0), Vec3(0)).lengthSqrt(), 0.0f);
}

TEST(GlobalIllumination, OpaqueGeometryOccludesSkyAndUsesWorldTransforms) {
  GlobalIllumination gi;
  gi.setSkyColor(Color(1));
  Transform transform;
  transform.position = Vec3(10, 0, 0);
  gi.addModel(box(Color(0)), transform);
  ASSERT_TRUE(gi.bake(Vec3(9, -1, -1), Vec3(11, 1, 1), 2, 2, 2, 128, 3));
  EXPECT_EQ(gi.sampleIrradiance(Vec3(10, 0, 0), Vec3(0, 1, 0)).lengthSqrt(),
            0.0f);
}

TEST(GlobalIllumination, MultipleBouncesTransferColorAndAreDeterministic) {
  GlobalIllumination gi;
  gi.addModel(box(Color(0.8f, 0.1f, 0.05f, 1.0f)), Transform{}, Color(1),
              Color(0.2f));
  ASSERT_TRUE(gi.bake(Vec3(-1), Vec3(1), 2, 2, 2, 1024, 1));
  const auto direct = gi.sampleIrradiance(Vec3(0), Vec3(0, 1, 0));
  ASSERT_TRUE(gi.bake(Vec3(-1), Vec3(1), 2, 2, 2, 1024, 3));
  const auto bounced = gi.sampleIrradiance(Vec3(0), Vec3(0, 1, 0));
  EXPECT_GT(bounced.x, direct.x * 2.0f);
  EXPECT_GT(bounced.x, bounced.y * 2.0f);
  EXPECT_GT(bounced.y, bounced.z);
  ASSERT_TRUE(gi.bake(Vec3(-1), Vec3(1), 2, 2, 2, 1024, 3));
  EXPECT_EQ(gi.sampleIrradiance(Vec3(0), Vec3(0, 1, 0)).x, bounced.x);
}

TEST(GlobalIllumination, ValidatesSettingsAndInvalidatesWhenSceneChanges) {
  GlobalIllumination gi;
  EXPECT_FALSE(gi.isBaked());
  ASSERT_TRUE(gi.bake(Vec3(-1), Vec3(1), 2, 2, 2, 32, 1));
  EXPECT_FALSE(gi.bake(Vec3(1), Vec3(-1)));
  EXPECT_FALSE(gi.bake(Vec3(-1), Vec3(1), 1));
  EXPECT_FALSE(gi.bake(Vec3(-1), Vec3(1), 2, 2, 33));
  EXPECT_FALSE(gi.bake(Vec3(-1), Vec3(1), 2, 2, 2, 31));
  EXPECT_FALSE(gi.bake(Vec3(-1), Vec3(1), 2, 2, 2, 32, 0));
  EXPECT_FALSE(gi.bake(Vec3(std::numeric_limits<float>::quiet_NaN()), Vec3(1)));
  EXPECT_TRUE(gi.isBaked());
  gi.setSkyColor(Color(1));
  EXPECT_FALSE(gi.isBaked());
  ASSERT_TRUE(gi.bake(Vec3(-1), Vec3(1), 2, 2, 2, 32, 1));
  gi.setLight(PointLight{});
  EXPECT_FALSE(gi.isBaked());
  gi.clear();
  EXPECT_EQ(gi.sampleIrradiance(Vec3(0), Vec3(0, 1, 0)).lengthSqrt(), 0.0f);
}
