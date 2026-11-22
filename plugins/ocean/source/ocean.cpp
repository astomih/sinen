#include <ocean/ocean.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <random>
#include <stdexcept>
#include <string>

namespace sinen::ocean {
namespace {
constexpr double pi = std::numbers::pi;
constexpr double tau = 2.0 * pi;

void range(double value, double lo, double hi, const char *name) {
  if (!std::isfinite(value) || value < lo || value > hi)
    throw std::invalid_argument(std::string("ocean: invalid ") + name);
}

void validate(const WaveSystem &s) {
  if (s.spectrum != Spectrum::Phillips && s.spectrum != Spectrum::Jonswap)
    throw std::invalid_argument("ocean: unknown spectrum");
  range(s.significantWaveHeight, 0, 100, "significantWaveHeight");
  range(s.windSpeed, 0.1, 100, "windSpeed");
  if (!std::isfinite(s.direction))
    throw std::invalid_argument("ocean: invalid direction");
  range(s.peakPeriod, 0.1, 100, "peakPeriod");
  range(s.peakEnhancement, 1, 20, "peakEnhancement");
  range(s.spread, 0, 128, "spread");
  range(s.shortWaveDamping, 0, 100, "shortWaveDamping");
}

int waveNumber(std::size_t i, std::size_t n) {
  return i < n / 2 ? static_cast<int>(i)
                   : static_cast<int>(i) - static_cast<int>(n);
}

double omega(double k, double g, double depth) {
  return std::sqrt(g * k * std::tanh(k * depth));
}

double rawDensity(const WaveSystem &s, double kx, double kz, double g,
                  double depth) {
  const double k = std::hypot(kx, kz);
  if (k == 0)
    return 0;
  const double alignment =
      (kx * std::cos(s.direction) + kz * std::sin(s.direction)) / k;
  const double damping =
      std::exp(-k * k * s.shortWaveDamping * s.shortWaveDamping);
  if (s.spectrum == Spectrum::Phillips) {
    const double length = s.windSpeed * s.windSpeed / g;
    // Tessendorf's Phillips k^-4 spectrum with a small opposing-wave lobe.
    return std::exp(-1.0 / (k * k * length * length)) * alignment * alignment *
           (alignment < 0 ? 0.07 : 1.0) * damping / (k * k * k * k);
  }
  if (alignment <= 0)
    return 0;
  const double w = omega(k, g, depth);
  const double peak = tau / s.peakPeriod;
  const double sigma = w <= peak ? 0.07 : 0.09;
  const double delta = (w - peak) / (sigma * peak);
  const double enhancement = std::exp(-0.5 * delta * delta);
  const double ratio = peak / w;
  const double spectrum = 0.0081 * g * g * std::pow(w, -5.0) *
                          std::exp(-1.25 * std::pow(ratio, 4.0)) *
                          std::pow(s.peakEnhancement, enhancement);
  // Convert S(omega) D(theta) d(omega) d(theta) to P(kx,kz) dkx dkz.
  const double kd = k * depth;
  const double th = std::tanh(kd);
  const double derivative = g * (th + kd * (1.0 - th * th)) / (2.0 * w);
  const double angularNorm = std::exp(std::lgamma((s.spread + 2) / 2) -
                                      std::lgamma((s.spread + 1) / 2)) /
                             std::sqrt(pi);
  return spectrum * angularNorm * std::pow(alignment, s.spread) * derivative /
         k * damping;
}
} // namespace

double Ocean::density(const WaveSystem &s, double kx, double kz, double gravity,
                      double depth) {
  validate(s);
  range(gravity, 0.1, 100, "gravity");
  range(depth, 0.1, 11000, "depth");
  if (!std::isfinite(kx) || !std::isfinite(kz))
    throw std::invalid_argument("ocean: invalid wave vector");
  return rawDensity(s, kx, kz, gravity, depth);
}

Ocean::Ocean(const Settings &settings) : config(settings) {
  const std::size_t n = config.resolution;
  if (n < 8 || n > 512 || (n & (n - 1)) != 0)
    throw std::invalid_argument(
        "ocean: resolution must be a power of two in [8, 512]");
  range(config.patchSize, 1, 1e6, "patchSize");
  range(config.depth, 0.1, 11000, "depth");
  range(config.gravity, 0.1, 100, "gravity");
  range(config.choppiness, 0, 4, "choppiness");
  range(config.foamThreshold, 0, 1, "foamThreshold");
  range(config.foamGain, 0, 100, "foamGain");
  validate(config.wind);
  validate(config.swell);
  initial.resize(n * n);
  heights.resize(n * n);
  displacementsX.resize(n * n);
  displacementsZ.resize(n * n);
  frequencies.resize(n * n);
  samples.resize(n * n);
  column.resize(n);
  roots.resize(n / 2);
  reversed.resize(n);
  for (std::size_t i = 0; i < n; ++i) {
    std::size_t x = i, r = 0;
    for (std::size_t b = 1; b < n; b <<= 1) {
      r = (r << 1) | (x & 1);
      x >>= 1;
    }
    reversed[i] = r;
  }
  for (std::size_t i = 0; i < n / 2; ++i)
    roots[i] = std::polar(1.0, tau * static_cast<double>(i) / n);

  const double dk = tau / config.patchSize;
  std::mt19937 random(config.seed);
  // Explicit Box-Muller mapping makes the seed independent of STL
  // normal_distribution.
  const auto gaussian = [&]() {
    const double u = (static_cast<double>(random()) + 0.5) / 4294967296.0;
    const double v = (static_cast<double>(random()) + 0.5) / 4294967296.0;
    return std::polar(std::sqrt(-2 * std::log(u)), tau * v);
  };
  std::uint32_t stream = 0;
  for (const auto &system : {config.wind, config.swell}) {
    random.seed(config.seed + stream++ * 0x9e3779b9u);
    if (system.significantWaveHeight == 0)
      continue;
    std::vector<double> power(n * n);
    double total = 0;
    for (std::size_t z = 0; z < n; ++z) {
      for (std::size_t x = 0; x < n; ++x) {
        // Remove DC and self-conjugate Nyquist axes so odd displacement spectra
        // remain Hermitian, without discarding imaginary residuals as a
        // workaround.
        if ((x == 0 && z == 0) || x == n / 2 || z == n / 2)
          continue;
        const double kx = dk * waveNumber(x, n), kz = dk * waveNumber(z, n);
        const std::size_t i = z * n + x;
        power[i] =
            rawDensity(system, kx, kz, config.gravity, config.depth) * dk * dk;
        total += power[i];
      }
    }
    if (!(total > std::numeric_limits<double>::min()))
      throw std::invalid_argument(
          "ocean: spectrum is unresolved; increase patchSize/resolution or "
          "change wave periods");
    const double variance =
        system.significantWaveHeight * system.significantWaveHeight / 16.0;
    for (std::size_t i = 0; i < initial.size(); ++i)
      initial[i] += gaussian() * (0.5 * std::sqrt(power[i] / total * variance));
  }
  for (std::size_t z = 0; z < n; ++z)
    for (std::size_t x = 0; x < n; ++x)
      frequencies[z * n + x] =
          omega(dk * std::hypot(waveNumber(x, n), waveNumber(z, n)),
                config.gravity, config.depth);
  update(0);
}

void Ocean::inverse1D(std::span<Complex> values) const {
  const auto n = values.size();
  for (std::size_t i = 0; i < n; ++i)
    if (i < reversed[i])
      std::swap(values[i], values[reversed[i]]);
  for (std::size_t length = 2; length <= n; length <<= 1) {
    const auto half = length / 2, stride = n / length;
    for (std::size_t start = 0; start < n; start += length) {
      for (std::size_t j = 0; j < half; ++j) {
        const auto a = values[start + j];
        const auto b = roots[j * stride] * values[start + j + half];
        values[start + j] = a + b;
        values[start + j + half] = a - b;
      }
    }
  }
  // Unnormalised inverse: coefficients are Fourier-series amplitudes in metres.
}

void Ocean::inverse2D(std::vector<Complex> &values) {
  const std::size_t n = config.resolution;
  for (std::size_t z = 0; z < n; ++z)
    inverse1D(std::span<Complex>(values.data() + z * n, n));
  for (std::size_t x = 0; x < n; ++x) {
    for (std::size_t z = 0; z < n; ++z)
      column[z] = values[z * n + x];
    inverse1D(column);
    for (std::size_t z = 0; z < n; ++z)
      values[z * n + x] = column[z];
  }
}

void Ocean::update(double timeSeconds) {
  if (!std::isfinite(timeSeconds))
    throw std::invalid_argument("ocean: time must be finite");
  const std::size_t n = config.resolution;
  for (std::size_t z = 0; z < n; ++z) {
    for (std::size_t x = 0; x < n; ++x) {
      const std::size_t i = z * n + x,
                        opposite = ((n - z) % n) * n + (n - x) % n;
      const double w = frequencies[i];
      const auto phase =
          std::polar(1.0, w == 0 ? 0 : -w * std::fmod(timeSeconds, tau / w));
      heights[i] = initial[i] * phase + std::conj(initial[opposite] * phase);
      const double kx = waveNumber(x, n), kz = waveNumber(z, n);
      const double k = std::hypot(kx, kz);
      // +i k/|k| makes crests converge for positive choppiness with exp(+ikx).
      displacementsX[i] =
          k == 0 ? Complex{}
                 : heights[i] * Complex(0, kx / k * config.choppiness);
      displacementsZ[i] =
          k == 0 ? Complex{}
                 : heights[i] * Complex(0, kz / k * config.choppiness);
    }
  }
  inverse2D(heights);
  inverse2D(displacementsX);
  inverse2D(displacementsZ);
  const double derivativeScale = n / (2.0 * config.patchSize);
  for (std::size_t z = 0; z < n; ++z) {
    for (std::size_t x = 0; x < n; ++x) {
      const auto i = z * n + x;
      const auto left = z * n + (x + n - 1) % n, right = z * n + (x + 1) % n;
      const auto back = ((z + n - 1) % n) * n + x,
                 front = ((z + 1) % n) * n + x;
      const auto derivative = [&](const auto &values, std::size_t a,
                                  std::size_t b) {
        return (values[b].real() - values[a].real()) * derivativeScale;
      };
      const double xx = 1 + derivative(displacementsX, left, right);
      const double xz = derivative(displacementsX, back, front);
      const double zx = derivative(displacementsZ, left, right);
      const double zz = 1 + derivative(displacementsZ, back, front);
      const double hx = derivative(heights, left, right),
                   hz = derivative(heights, back, front);
      const double jacobian = xx * zz - xz * zx;
      double nx = hz * zx - zz * hx, ny = jacobian, nz = xz * hx - hz * xx;
      const double length = std::sqrt(nx * nx + ny * ny + nz * nz);
      if (length > 1e-12) {
        nx /= length;
        ny /= length;
        nz /= length;
      } else {
        nx = nz = 0;
        ny = 1;
      }
      samples[i] = {
          static_cast<float>(displacementsX[i].real()),
          static_cast<float>(heights[i].real()),
          static_cast<float>(displacementsZ[i].real()),
          static_cast<float>(nx),
          static_cast<float>(ny),
          static_cast<float>(nz),
          static_cast<float>(std::clamp(
              (config.foamThreshold - jacobian) * config.foamGain, 0.0, 1.0))};
    }
  }
}

SurfaceSample Ocean::sample(double x, double z) const {
  if (!std::isfinite(x) || !std::isfinite(z))
    throw std::invalid_argument("ocean: sample coordinates must be finite");
  const std::size_t n = config.resolution;
  const auto coord = [&](double value) {
    double wrapped = std::fmod(value, config.patchSize);
    if (wrapped < 0)
      wrapped += config.patchSize;
    return wrapped * n / config.patchSize;
  };
  const double gx = coord(x), gz = coord(z);
  const auto ix = static_cast<std::size_t>(gx) % n,
             iz = static_cast<std::size_t>(gz) % n;
  const float tx = static_cast<float>(gx - std::floor(gx)),
              tz = static_cast<float>(gz - std::floor(gz));
  const auto &a = samples[iz * n + ix], &b = samples[iz * n + (ix + 1) % n];
  const auto &c = samples[((iz + 1) % n) * n + ix],
             &d = samples[((iz + 1) % n) * n + (ix + 1) % n];
  const auto blend = [&](float SurfaceSample::*field) {
    return std::lerp(std::lerp(a.*field, b.*field, tx),
                     std::lerp(c.*field, d.*field, tx), tz);
  };
  SurfaceSample result{
      blend(&SurfaceSample::displacementX), blend(&SurfaceSample::height),
      blend(&SurfaceSample::displacementZ), blend(&SurfaceSample::normalX),
      blend(&SurfaceSample::normalY),       blend(&SurfaceSample::normalZ),
      blend(&SurfaceSample::foam)};
  const float length = std::sqrt(result.normalX * result.normalX +
                                 result.normalY * result.normalY +
                                 result.normalZ * result.normalZ);
  if (length > 1e-6f) {
    result.normalX /= length;
    result.normalY /= length;
    result.normalZ /= length;
  } else {
    result.normalX = result.normalZ = 0;
    result.normalY = 1;
  }
  return result;
}

std::size_t Ocean::vertexCount() const {
  const std::size_t side = config.resolution + 1;
  return side * side;
}
std::size_t Ocean::indexCount() const {
  return std::size_t(config.resolution) * config.resolution * 6;
}

void Ocean::writeVertices(std::span<Vertex> output) const {
  if (output.size() != vertexCount())
    throw std::invalid_argument("ocean: incorrect vertex buffer size");
  const std::size_t n = config.resolution;
  for (std::size_t z = 0; z <= n; ++z) {
    for (std::size_t x = 0; x <= n; ++x) {
      const auto &s = samples[(z % n) * n + x % n];
      const float u = static_cast<float>(x) / n, v = static_cast<float>(z) / n;
      output[z * (n + 1) + x] = {
          static_cast<float>(u * config.patchSize + s.displacementX),
          s.height,
          static_cast<float>(v * config.patchSize + s.displacementZ),
          s.normalX,
          s.normalY,
          s.normalZ,
          u,
          v,
          s.foam,
          0,
          0,
          1};
    }
  }
}

void Ocean::writeIndices(std::span<std::uint32_t> output) const {
  if (output.size() != indexCount())
    throw std::invalid_argument("ocean: incorrect index buffer size");
  const std::uint32_t n = config.resolution, side = n + 1;
  std::size_t i = 0;
  for (std::uint32_t z = 0; z < n; ++z) {
    for (std::uint32_t x = 0; x < n; ++x) {
      const auto a = z * side + x, b = a + 1, c = a + side, d = c + 1;
      for (auto index : {a, c, b, b, c, d})
        output[i++] = index;
    }
  }
}
} // namespace sinen::ocean
