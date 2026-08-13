#include "debug_overlay.hpp"

#include <gpu/gpu_device.hpp>
#include <graphics/graphics.hpp>
#include <math/geometry/rect.hpp>
#include <platform/window/window.hpp>

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <string_view>

namespace sinen {
namespace {
constexpr uint32_t overlayWidth = 512;
constexpr uint32_t overlayHeight = 138;
constexpr int glyphWidth = 5;
constexpr int glyphHeight = 7;
constexpr int glyphScale = 2;
constexpr int glyphAdvance = (glyphWidth + 1) * glyphScale;
constexpr int lineAdvance = (glyphHeight + 2) * glyphScale;
constexpr int textLeft = 8;
constexpr int textTop = 7;
constexpr float refreshInterval = 0.5f;

using Glyph = std::array<uint8_t, glyphHeight>;

constexpr Glyph glyph(char character) {
  switch (character) {
  case 'A':
    return {14, 17, 17, 31, 17, 17, 17};
  case 'B':
    return {30, 17, 17, 30, 17, 17, 30};
  case 'C':
    return {14, 17, 16, 16, 16, 17, 14};
  case 'D':
    return {30, 17, 17, 17, 17, 17, 30};
  case 'E':
    return {31, 16, 16, 30, 16, 16, 31};
  case 'F':
    return {31, 16, 16, 30, 16, 16, 16};
  case 'G':
    return {14, 17, 16, 23, 17, 17, 15};
  case 'H':
    return {17, 17, 17, 31, 17, 17, 17};
  case 'I':
    return {31, 4, 4, 4, 4, 4, 31};
  case 'J':
    return {7, 2, 2, 2, 18, 18, 12};
  case 'K':
    return {17, 18, 20, 24, 20, 18, 17};
  case 'L':
    return {16, 16, 16, 16, 16, 16, 31};
  case 'M':
    return {17, 27, 21, 21, 17, 17, 17};
  case 'N':
    return {17, 25, 21, 19, 17, 17, 17};
  case 'O':
    return {14, 17, 17, 17, 17, 17, 14};
  case 'P':
    return {30, 17, 17, 30, 16, 16, 16};
  case 'Q':
    return {14, 17, 17, 17, 21, 18, 13};
  case 'R':
    return {30, 17, 17, 30, 20, 18, 17};
  case 'S':
    return {15, 16, 16, 14, 1, 1, 30};
  case 'T':
    return {31, 4, 4, 4, 4, 4, 4};
  case 'U':
    return {17, 17, 17, 17, 17, 17, 14};
  case 'V':
    return {17, 17, 17, 17, 17, 10, 4};
  case 'W':
    return {17, 17, 17, 21, 21, 21, 10};
  case 'X':
    return {17, 17, 10, 4, 10, 17, 17};
  case 'Y':
    return {17, 17, 10, 4, 4, 4, 4};
  case 'Z':
    return {31, 1, 2, 4, 8, 16, 31};
  case '0':
    return {14, 17, 19, 21, 25, 17, 14};
  case '1':
    return {4, 12, 4, 4, 4, 4, 14};
  case '2':
    return {14, 17, 1, 2, 4, 8, 31};
  case '3':
    return {30, 1, 1, 14, 1, 1, 30};
  case '4':
    return {2, 6, 10, 18, 31, 2, 2};
  case '5':
    return {31, 16, 16, 30, 1, 1, 30};
  case '6':
    return {14, 16, 16, 30, 17, 17, 14};
  case '7':
    return {31, 1, 2, 4, 8, 8, 8};
  case '8':
    return {14, 17, 17, 14, 17, 17, 14};
  case '9':
    return {14, 17, 17, 15, 1, 1, 14};
  case ':':
    return {0, 4, 4, 0, 4, 4, 0};
  case '.':
    return {0, 0, 0, 0, 0, 6, 6};
  case '-':
    return {0, 0, 0, 31, 0, 0, 0};
  case '_':
    return {0, 0, 0, 0, 0, 0, 31};
  case '/':
    return {1, 1, 2, 4, 8, 16, 16};
  case '|':
    return {4, 4, 4, 4, 4, 4, 4};
  case '(':
    return {2, 4, 8, 8, 8, 4, 2};
  case ')':
    return {8, 4, 2, 2, 2, 4, 8};
  case '[':
    return {14, 8, 8, 8, 8, 8, 14};
  case ']':
    return {14, 2, 2, 2, 2, 2, 14};
  case '+':
    return {0, 4, 4, 31, 4, 4, 0};
  case '=':
    return {0, 0, 31, 0, 31, 0, 0};
  case '%':
    return {17, 2, 4, 8, 17, 0, 0};
  case '?':
    return {14, 17, 1, 2, 4, 0, 4};
  case ' ':
    return {0, 0, 0, 0, 0, 0, 0};
  default:
    return {14, 17, 1, 2, 4, 0, 4};
  }
}

std::string asciiUpper(StringView text) {
  std::string result;
  result.reserve(text.size());
  for (size_t i = 0; i < text.size(); ++i) {
    const auto byte = static_cast<unsigned char>(text[i]);
    if (byte < 0x80) {
      result.push_back(static_cast<char>(std::toupper(byte)));
    } else if ((byte & 0xC0) != 0x80) {
      result.push_back('?');
    }
  }
  return result;
}

void setPixel(std::vector<uint8_t> &pixels, int x, int y, uint8_t red,
              uint8_t green, uint8_t blue, uint8_t alpha) {
  if (x < 0 || y < 0 || x >= static_cast<int>(overlayWidth) ||
      y >= static_cast<int>(overlayHeight)) {
    return;
  }
  const size_t index =
      (static_cast<size_t>(y) * overlayWidth + static_cast<size_t>(x)) * 4;
  pixels[index + 0] = red;
  pixels[index + 1] = green;
  pixels[index + 2] = blue;
  pixels[index + 3] = alpha;
}

void drawLine(std::vector<uint8_t> &pixels, StringView text, int line,
              bool heading) {
  const int maxCharacters =
      (static_cast<int>(overlayWidth) - textLeft * 2) / glyphAdvance;
  const int count = std::min(static_cast<int>(text.size()), maxCharacters);
  const int yBase = textTop + line * lineAdvance;
  const uint8_t red = heading ? 255 : 235;
  const uint8_t green = heading ? 240 : 235;
  const uint8_t blue = heading ? 110 : 235;
  for (int i = 0; i < count; ++i) {
    const Glyph rows = glyph(text[static_cast<size_t>(i)]);
    const int xBase = textLeft + i * glyphAdvance;
    for (int y = 0; y < glyphHeight; ++y) {
      for (int x = 0; x < glyphWidth; ++x) {
        if ((rows[static_cast<size_t>(y)] & (1u << (glyphWidth - x - 1))) ==
            0) {
          continue;
        }
        for (int sy = 0; sy < glyphScale; ++sy) {
          for (int sx = 0; sx < glyphScale; ++sx) {
            setPixel(pixels, xBase + x * glyphScale + sx,
                     yBase + y * glyphScale + sy, red, green, blue, 255);
          }
        }
      }
    }
  }
}

std::string formatted(const char *format, double first, double second) {
  std::array<char, 96> buffer{};
  std::snprintf(buffer.data(), buffer.size(), format, first, second);
  return buffer.data();
}
} // namespace

void DebugOverlay::initialize() {
  auto device = Graphics::getDevice();
  gpuName = device ? device->getName() : String("UNKNOWN");
  gpuDriver = device ? device->getDriver() : String("UNKNOWN");
  platform = SDL_GetPlatform() ? SDL_GetPlatform() : "UNKNOWN";
  cpuCores = SDL_GetNumLogicalCPUCores();
  systemRamMiB = SDL_GetSystemRAM();
  texture.reset();
  textureDirty = true;
  Graphics::addOverlayDrawFunc([this] { draw(); });
}

void DebugOverlay::shutdown() { texture.reset(); }

void DebugOverlay::update(float deltaTime, bool togglePressed) {
  if (togglePressed) {
    visible = !visible;
    textureDirty = visible;
  }

  if (std::isfinite(deltaTime) && deltaTime > 0.0f && deltaTime < 1.0f) {
    sampleSeconds += deltaTime;
    ++sampleFrames;
  }
  if (sampleSeconds >= refreshInterval && sampleFrames > 0) {
    displayedFps = static_cast<float>(sampleFrames) / sampleSeconds;
    displayedFrameMs =
        sampleSeconds * 1000.0f / static_cast<float>(sampleFrames);
    sampleSeconds = 0.0f;
    sampleFrames = 0;
    textureDirty = textureDirty || visible;
  }

  if (visible && textureDirty) {
    if (displayedFps <= 0.0f && deltaTime > 0.0f) {
      displayedFps = 1.0f / deltaTime;
      displayedFrameMs = deltaTime * 1000.0f;
    }
    refreshTexture();
  }
}

void DebugOverlay::refreshTexture() {
  const Vec2 windowSize = Window::size();
  std::array<std::string, 7> lines{
      "SINEN DEBUG OVERLAY (F3)",
      formatted("FPS %.1F | FRAME %.2F MS", displayedFps, displayedFrameMs),
      "WINDOW " + std::to_string(static_cast<int>(windowSize.x)) + " X " +
          std::to_string(static_cast<int>(windowSize.y)) + " | MSAA " +
          std::to_string(Graphics::getMSAASampleCount()) + "X",
      "GPU " + asciiUpper(gpuName),
      "API " + asciiUpper(Graphics::getBackendName()) + " | DRIVER " +
          asciiUpper(gpuDriver),
      "CPU " + std::to_string(cpuCores) + " LOGICAL CORES | RAM " +
          std::to_string(systemRamMiB) + " MB",
      "OS " + asciiUpper(platform),
  };

  pixelBuffer.resize(static_cast<size_t>(overlayWidth) * overlayHeight * 4);
  std::fill(pixelBuffer.begin(), pixelBuffer.end(), 0);
  for (uint32_t y = 0; y < overlayHeight; ++y) {
    for (uint32_t x = 0; x < overlayWidth; ++x) {
      const bool border =
          x == 0 || y == 0 || x + 1 == overlayWidth || y + 1 == overlayHeight;
      setPixel(pixelBuffer, static_cast<int>(x), static_cast<int>(y),
               border ? 110 : 0, border ? 110 : 0, border ? 110 : 0,
               border ? 220 : 178);
    }
  }
  for (size_t line = 0; line < lines.size(); ++line) {
    drawLine(pixelBuffer, lines[line], static_cast<int>(line), line == 0);
  }

  if (!texture) {
    texture = Texture::create();
  }
  texture->updatePixels(pixelBuffer.data(), overlayWidth, overlayHeight,
                        gpu::TextureFormat::R8G8B8A8_UNORM, 4);
  textureDirty = false;
}

void DebugOverlay::draw() {
  if (!visible || !texture) {
    return;
  }
  Graphics::begin2D();
  Graphics::drawOverlayImage(
      texture, Rect(Vec2(8.0f, 8.0f), Vec2(static_cast<float>(overlayWidth),
                                           static_cast<float>(overlayHeight))));
}
} // namespace sinen
