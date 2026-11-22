#pragma once

#include <complex>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace sinen::ocean {

enum class Spectrum { Phillips, Jonswap };

struct WaveSystem {
  Spectrum spectrum = Spectrum::Jonswap;
  double significantWaveHeight = 2.0; // metres; 4 * sqrt(expected variance)
  double windSpeed = 12.0;            // m/s (Phillips length scale)
  double direction = 0.0;             // radians, +X towards +Z, propagation
  double peakPeriod = 7.0;            // seconds (JONSWAP)
  double peakEnhancement = 3.3;       // gamma (JONSWAP)
  double spread = 4.0;                // cosine power in the forward hemisphere
  double shortWaveDamping = 0.1;      // metres
};

struct Settings {
  std::uint32_t resolution = 128; // power of two, [8, 512]
  double patchSize = 256.0;       // metres, periodic XZ domain
  double depth = 200.0;           // metres, finite-depth dispersion
  double gravity = 9.81;
  double choppiness = 1.0;
  double foamThreshold = 0.8; // compression Jacobian threshold
  double foamGain = 2.0;
  std::uint32_t seed = 1;
  WaveSystem wind;
  WaveSystem swell{Spectrum::Jonswap, 1.0, 12.0, 0.6, 12.0, 5.0, 16.0, 0.1};
};

struct SurfaceSample {
  float displacementX = 0, height = 0, displacementZ = 0;
  float normalX = 0, normalY = 1, normalZ = 0;
  float foam = 0;
};

// Binary mesh layout, independent of engine types: position, normal, UV, RGBA.
// Foam is in color.r; alpha is always one. Indices are zero-based uint32.
struct Vertex {
  float x, y, z, nx, ny, nz, u, v, r, g, b, a;
};
static_assert(sizeof(Vertex) == 48);

// CPU radix-2 FFT, O(N^2 log N), with persistent work buffers. No GPU required.
class Ocean {
public:
  explicit Ocean(const Settings &settings = {});
  void update(double timeSeconds); // absolute time, deterministic and seekable
  const Settings &settings() const { return config; }
  std::span<const SurfaceSample> surface() const { return samples; }
  // Coordinates are undisplaced XZ positions in metres. Wraps periodically.
  SurfaceSample sample(double x, double z) const;
  std::size_t vertexCount() const;
  std::size_t indexCount() const;
  void writeVertices(std::span<Vertex> output) const;
  void writeIndices(std::span<std::uint32_t> output) const;

  // Spectral densities before discrete Hs normalization (for inspection).
  static double density(const WaveSystem &system, double kx, double kz,
                        double gravity = 9.81, double depth = 200.0);

private:
  using Complex = std::complex<double>;
  Settings config;
  std::vector<Complex> initial, heights, displacementsX, displacementsZ;
  std::vector<double> frequencies;
  std::vector<SurfaceSample> samples;
  std::vector<std::size_t> reversed;
  std::vector<Complex> roots, column;
  void inverse1D(std::span<Complex> values) const;
  void inverse2D(std::vector<Complex> &values);
};

} // namespace sinen::ocean
