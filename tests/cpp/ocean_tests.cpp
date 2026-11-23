#include <gtest/gtest.h>
#include <ocean/ocean.hpp>

#include <cmath>
#include <complex>
#include <limits>
#include <numbers>
#include <vector>

using namespace sinen::ocean;
namespace {
constexpr double tau = 2 * std::numbers::pi;
double variance(const Ocean &ocean) {
  double sum = 0;
  for (const auto &s : ocean.surface())
    sum += s.height * s.height;
  return sum / ocean.surface().size();
}
std::complex<double> mode(const Ocean &ocean, int kx, int kz) {
  std::complex<double> result{};
  const int n = ocean.settings().resolution;
  for (int z = 0; z < n; ++z)
    for (int x = 0; x < n; ++x)
      result += double(ocean.surface()[z * n + x].height) *
                std::polar(1.0, -tau * (kx * x + kz * z) / n);
  return result / double(n * n);
}
} // namespace

TEST(Ocean, RejectsInvalidSettingsAndNonfiniteInputs) {
  Settings s;
  s.resolution = 63;
  EXPECT_THROW(Ocean{s}, std::invalid_argument);
  s.resolution = 1024;
  EXPECT_THROW(Ocean{s}, std::invalid_argument);
  s.resolution = 8;
  s.patchSize = 0;
  EXPECT_THROW(Ocean{s}, std::invalid_argument);
  s.patchSize = 256;
  s.wind.peakEnhancement = 0;
  EXPECT_THROW(Ocean{s}, std::invalid_argument);
  s.wind.peakEnhancement = 3.3;
  s.swell.direction = std::numeric_limits<double>::quiet_NaN();
  EXPECT_THROW(Ocean{s}, std::invalid_argument);
  Ocean ocean;
  EXPECT_THROW(ocean.update(std::numeric_limits<double>::infinity()),
               std::invalid_argument);
  EXPECT_THROW(ocean.sample(NAN, 0), std::invalid_argument);
  std::vector<Vertex> small(1);
  EXPECT_THROW(ocean.writeVertices(small), std::invalid_argument);
}

TEST(Ocean, BothSpectraAnimateWithDeterministicSeedAndZeroMean) {
  for (const auto spectrum : {Spectrum::Phillips, Spectrum::Jonswap}) {
    Settings s;
    s.resolution = 32;
    s.wind.spectrum = spectrum;
    Ocean a(s), b(s);
    const float before = a.surface()[17].height;
    a.update(3.75);
    b.update(3.75);
    EXPECT_NE(a.surface()[17].height, before);
    double mean = 0;
    for (std::size_t i = 0; i < a.surface().size(); ++i) {
      const auto &v = a.surface()[i];
      EXPECT_EQ(v.height, b.surface()[i].height);
      EXPECT_EQ(v.displacementX, b.surface()[i].displacementX);
      EXPECT_NEAR(v.normalX * v.normalX + v.normalY * v.normalY +
                      v.normalZ * v.normalZ,
                  1, 2e-6);
      EXPECT_GE(v.foam, 0);
      EXPECT_LE(v.foam, 1);
      mean += v.height;
    }
    EXPECT_NEAR(mean / a.surface().size(), 0, 1e-7);
    const auto atTime = a.surface()[17];
    a.update(-100);
    a.update(3.75);
    EXPECT_EQ(a.surface()[17].height, atTime.height);
    s.seed++;
    Ocean c(s);
    c.update(3.75);
    EXPECT_NE(c.surface()[17].height, atTime.height);
  }
}

TEST(Ocean, FourierModesFollowFiniteDepthDispersion) {
  Settings s;
  s.resolution = 8;
  s.depth = 3;
  s.swell.significantWaveHeight = 0;
  Ocean ocean(s);
  // A direct DFT checks FFT bin ordering, phase, and the physical time
  // equation.
  for (const auto k : {std::pair{1, 0}, std::pair{0, 1}, std::pair{2, -1}}) {
    constexpr double t = 2.3, dt = 0.7;
    ocean.update(t - dt);
    const auto minus = mode(ocean, k.first, k.second);
    ocean.update(t);
    const auto center = mode(ocean, k.first, k.second);
    ocean.update(t + dt);
    const auto plus = mode(ocean, k.first, k.second);
    const double wave = tau / s.patchSize * std::hypot(k.first, k.second);
    const double w = std::sqrt(s.gravity * wave * std::tanh(wave * s.depth));
    EXPECT_LT(std::abs(plus + minus - 2 * std::cos(w * dt) * center), 2e-7);
  }
  EXPECT_LT(std::abs(mode(ocean, 0, 0)), 1e-7);
  EXPECT_LT(std::abs(mode(ocean, 4, 0)), 1e-7);
}

TEST(Ocean, WaveHeightIsIndependentOfFFTResolutionInExpectation) {
  for (const auto spectrum : {Spectrum::Phillips, Spectrum::Jonswap}) {
    for (const auto n : {32u, 128u}) {
      double energy = 0;
      Settings s;
      s.resolution = n;
      s.wind.spectrum = spectrum;
      s.wind.significantWaveHeight = 2;
      s.swell.significantWaveHeight = 0;
      for (unsigned seed = 0; seed < 24; ++seed) {
        s.seed = seed;
        Ocean ocean(s);
        ocean.update(7.1);
        energy += variance(ocean);
      }
      EXPECT_NEAR(energy / 24, 4.0 / 16, 0.065);
    }
  }
}

TEST(Ocean, IndependentSwellAndCalmSea) {
  Settings combinedSettings;
  combinedSettings.resolution = 32;
  Ocean combined(combinedSettings);
  auto windSettings = combinedSettings;
  windSettings.swell.significantWaveHeight = 0;
  auto swellSettings = combinedSettings;
  swellSettings.wind.significantWaveHeight = 0;
  Ocean windOnly(windSettings), swellOnly(swellSettings);
  for (std::size_t i = 0; i < combined.surface().size(); ++i)
    EXPECT_NEAR(combined.surface()[i].height,
                windOnly.surface()[i].height + swellOnly.surface()[i].height,
                2e-6);
  Settings s;
  s.resolution = 32;
  s.wind.significantWaveHeight = 0;
  s.swell.significantWaveHeight = 2;
  Ocean swell(s);
  EXPECT_GT(variance(swell), 0.01);
  s.swell.significantWaveHeight = 0;
  Ocean calm(s);
  calm.update(23);
  for (const auto &v : calm.surface()) {
    EXPECT_EQ(v.height, 0);
    EXPECT_EQ(v.displacementX, 0);
    EXPECT_EQ(v.displacementZ, 0);
    EXPECT_EQ(v.normalY, 1);
    EXPECT_EQ(v.foam, 0);
  }
}

TEST(Ocean, JonswapPeakAndDirectionalDensity) {
  WaveSystem s;
  s.shortWaveDamping = 0;
  s.direction = 0;
  s.peakPeriod = 8;
  const double w = tau / s.peakPeriod, k = w * w / 9.81;
  const double enhanced = Ocean::density(s, k, 0);
  s.peakEnhancement = 1;
  const double plain = Ocean::density(s, k, 0);
  EXPECT_NEAR(enhanced / plain, 3.3, 1e-6);
  EXPECT_EQ(Ocean::density(s, -k, 0), 0);
  EXPECT_EQ(Ocean::density(s, 0, 0), 0);
  s.direction = std::numbers::pi / 2;
  EXPECT_NEAR(Ocean::density(s, 0, k), plain, plain * 1e-10);
  s.spectrum = Spectrum::Phillips;
  const double forward = Ocean::density(s, 0, k);
  EXPECT_NEAR(Ocean::density(s, 0, -k) / forward, 0.07, 1e-10);
}

TEST(Ocean, MeshTilesExactlyAndTrianglesFaceUp) {
  Settings s;
  s.resolution = 16;
  s.choppiness = 0;
  Ocean ocean(s);
  ocean.update(1.2);
  std::vector<Vertex> vertices(ocean.vertexCount());
  ocean.writeVertices(vertices);
  std::vector<std::uint32_t> indices(ocean.indexCount());
  ocean.writeIndices(indices);
  for (unsigned i = 0; i <= s.resolution; ++i) {
    const auto &left = vertices[i * 17], &right = vertices[i * 17 + 16];
    EXPECT_EQ(left.y, right.y);
    EXPECT_EQ(left.nx, right.nx);
    EXPECT_NEAR(right.x - left.x, s.patchSize, 1e-5);
    const auto &back = vertices[i], &front = vertices[16 * 17 + i];
    EXPECT_EQ(back.y, front.y);
    EXPECT_EQ(back.nz, front.nz);
  }
  for (std::size_t i = 0; i < indices.size(); i += 3) {
    ASSERT_LT(indices[i + 2], vertices.size());
    const auto &a = vertices[indices[i]], &b = vertices[indices[i + 1]],
               &c = vertices[indices[i + 2]];
    EXPECT_GT((b.z - a.z) * (c.x - a.x) - (b.x - a.x) * (c.z - a.z), 0);
  }
  const auto a = ocean.sample(-0.3, 25.7),
             b = ocean.sample(s.patchSize - 0.3, 25.7 + s.patchSize);
  EXPECT_FLOAT_EQ(a.height, b.height);
  EXPECT_FLOAT_EQ(a.normalY, b.normalY);
}

TEST(Ocean, ChoppinessChangesGeometryAndCompressionFoam) {
  Settings s;
  s.resolution = 32;
  s.wind.significantWaveHeight = 8;
  s.foamThreshold = 1;
  s.choppiness = 0;
  Ocean smooth(s);
  s.choppiness = 2;
  Ocean choppy(s);
  double displacement = 0, foam = 0, crestCompression = 0;
  for (std::size_t i = 0; i < smooth.surface().size(); ++i) {
    EXPECT_EQ(smooth.surface()[i].height, choppy.surface()[i].height);
    EXPECT_EQ(smooth.surface()[i].foam, 0);
    displacement += std::abs(choppy.surface()[i].displacementX);
    foam += choppy.surface()[i].foam;
    const std::size_t n = s.resolution, x = i % n, z = i / n;
    const auto &left = choppy.surface()[z * n + (x + n - 1) % n];
    const auto &right = choppy.surface()[z * n + (x + 1) % n];
    crestCompression +=
        choppy.surface()[i].height * (right.displacementX - left.displacementX);
  }
  EXPECT_GT(displacement, 1);
  EXPECT_GT(foam, 1);
  EXPECT_LT(crestCompression, 0); // horizontal motion compresses crests
}
