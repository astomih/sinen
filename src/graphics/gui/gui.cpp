#include <graphics/gui/gui.hpp>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <format>
#include <functional>
#include <iterator>
#include <variant>
#include <vector>

#include <graphics/graphics.hpp>
#include <platform/input/mouse.hpp>
#include <platform/window/window.hpp>

namespace sinen {
namespace {
struct GuiTheme {
  Color background = Color(0.12f, 0.13f, 0.15f, 0.94f);
  Color hover = Color(0.22f, 0.24f, 0.28f, 0.96f);
  Color active = Color(0.30f, 0.34f, 0.42f, 0.98f);
  Color text = Color(1.0f);
  Color accent = Color(0.25f, 0.55f, 0.95f, 1.0f);
};

static GuiTheme theme;
static Ptr<Font> defaultFont;
static Ptr<Font> currentFont;
static float currentFontSize = 16.0f;
static std::uint64_t activeId = 0;
static std::uint64_t hotId = 0;

struct RectDrawCommand {
  Rect rect;
  Color color;
  std::uint8_t layer;
};

struct TextDrawCommand {
  String text;
  Ptr<Font> font;
  Color color;
  float fontSize;
  TextTransform transform;
};

using DrawCommand = std::variant<RectDrawCommand, TextDrawCommand>;
static std::vector<DrawCommand> drawCommands;

static std::uint64_t hashId(StringView text, const Rect &rect) {
  std::size_t seed =
      std::hash<std::string_view>{}(std::string_view(text.data(), text.size()));
  auto combine = [&](float value) {
    std::uint32_t bits = 0;
    static_assert(sizeof(bits) == sizeof(value));
    std::memcpy(&bits, &value, sizeof(bits));
    seed ^=
        static_cast<std::size_t>(bits) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  };
  combine(rect.x);
  combine(rect.y);
  combine(rect.width);
  combine(rect.height);
  return static_cast<std::uint64_t>(seed);
}

static Font *font() {
  if (currentFont && currentFont->isLoaded()) {
    return currentFont.get();
  }
  if (!defaultFont) {
    defaultFont = Font::create();
    defaultFont->load(16);
  }
  return defaultFont.get();
}

static Ptr<Font> fontPtr() {
  font();
  if (currentFont && currentFont->isLoaded()) {
    return currentFont;
  }
  return defaultFont;
}

static void queueRect(const Rect &rect, const Color &color,
                      std::uint8_t layer = 0) {
  drawCommands.emplace_back(RectDrawCommand{rect, color, layer});
}

static void queueText(StringView text, const Ptr<Font> &font,
                      const Color &color, float fontSize,
                      const TextTransform &transform) {
  drawCommands.emplace_back(TextDrawCommand{String(text), font, color,
                                             fontSize, transform});
}

static Vec2 mousePosition2D() {
  const Vec2 mouse = Mouse::getPosition();
  const Vec2 half = Window::half();
  const Vec2 windowPosition(half.x + mouse.x, half.y - mouse.y);
  return Graphics::windowToCurrent2D(windowPosition);
}

static bool contains(const Rect &rect, const Vec2 &point) {
  return point.x >= rect.x && point.x <= rect.x + rect.width &&
         point.y >= rect.y && point.y <= rect.y + rect.height;
}

static float fitFontSize(StringView text, const Rect &rect, float fontSize) {
  auto *f = font();
  if (f == nullptr || !f->isLoaded()) {
    return fontSize;
  }

  const float maxWidth = std::max(1.0f, rect.width - 16.0f);
  const float maxHeight = std::max(1.0f, rect.height - 6.0f);
  float size = std::min(fontSize, maxHeight);
  while (size > 8.0f) {
    const Rect region =
        f->region(text, static_cast<int>(size), Pivot::Center, rect.center());
    if (region.width <= maxWidth && region.height <= maxHeight) {
      break;
    }
    size -= 1.0f;
  }
  return size;
}

static Color widgetColor(std::uint64_t id, bool hovered) {
  if (activeId == id) {
    return theme.active;
  }
  return hovered ? theme.hover : theme.background;
}

static bool updateButtonState(std::uint64_t id, bool hovered) {
  if (hovered) {
    hotId = id;
  }
  if (hovered && Mouse::isPressed(Mouse::LEFT)) {
    activeId = id;
  }
  const bool clicked =
      (activeId == id && hovered && Mouse::isReleased(Mouse::LEFT));
  if (activeId == id && Mouse::isReleased(Mouse::LEFT)) {
    activeId = 0;
  }
  return clicked;
}
} // namespace

void Gui::shutdown() {
  drawCommands.clear();
  currentFont.reset();
  defaultFont.reset();
  activeId = 0;
  hotId = 0;
}

void Gui::newFrame() {
  drawCommands.clear();
  hotId = 0;
  if (!Mouse::isDown(Mouse::LEFT) && !Mouse::isReleased(Mouse::LEFT)) {
    activeId = 0;
  }
}

void Gui::render() {
  struct RectBatch {
    Color color;
    Array<Rect> rects;
  };

  std::uint8_t maxLayer = 0;
  for (const auto &command : drawCommands) {
    if (const auto *rect = std::get_if<RectDrawCommand>(&command)) {
      maxLayer = std::max(maxLayer, rect->layer);
    }
  }
  for (unsigned int layer = 0; layer <= maxLayer; ++layer) {
    std::vector<RectBatch> batches;
    for (const auto &command : drawCommands) {
      const auto *rect = std::get_if<RectDrawCommand>(&command);
      if (rect == nullptr || rect->layer != layer) {
        continue;
      }
      auto batch = std::find_if(
          batches.begin(), batches.end(), [&](const RectBatch &candidate) {
            return candidate.color.r == rect->color.r &&
                   candidate.color.g == rect->color.g &&
                   candidate.color.b == rect->color.b &&
                   candidate.color.a == rect->color.a;
          });
      if (batch == batches.end()) {
        batches.push_back(RectBatch{rect->color, {}});
        batch = std::prev(batches.end());
      }
      batch->rects.push_back(rect->rect);
    }
    for (const auto &batch : batches) {
      Graphics::drawRects(batch.rects, batch.color);
    }
  }

  struct TextBatch {
    Ptr<Font> font;
    Color color;
    float fontSize;
    Array<TextBatchItem> items;
  };
  std::vector<TextBatch> textBatches;
  for (const auto &command : drawCommands) {
    const auto *text = std::get_if<TextDrawCommand>(&command);
    if (text == nullptr) {
      continue;
    }
    auto batch = std::find_if(
        textBatches.begin(), textBatches.end(), [&](const TextBatch &candidate) {
          return candidate.font == text->font &&
                 candidate.fontSize == text->fontSize &&
                 candidate.color.r == text->color.r &&
                 candidate.color.g == text->color.g &&
                 candidate.color.b == text->color.b &&
                 candidate.color.a == text->color.a;
        });
    if (batch == textBatches.end()) {
      textBatches.push_back(
          TextBatch{text->font, text->color, text->fontSize, {}});
      batch = std::prev(textBatches.end());
    }
    batch->items.push_back(TextBatchItem{text->text, text->transform});
  }
  for (const auto &batch : textBatches) {
    Graphics::drawTexts(
        batch.items,
        TextStyle(batch.font, batch.color, batch.fontSize));
  }
  drawCommands.clear();
}

void Gui::setFont(const Ptr<Font> &font) { currentFont = font; }

void Gui::setFontSize(float size) {
  if (size > 0.0f) {
    currentFontSize = size;
  }
}

float Gui::fontSize() { return currentFontSize; }

void Gui::setThemeColor(const Color &background, const Color &hover,
                        const Color &active, const Color &text,
                        const Color &accent) {
  theme.background = background;
  theme.hover = hover;
  theme.active = active;
  theme.text = text;
  theme.accent = accent;
}

Color Gui::textColor() { return theme.text; }

void Gui::label(StringView text, const Vec2 &position, const Color &color,
                float fontSize) {
  auto *f = font();
  if (f == nullptr || !f->isLoaded()) {
    return;
  }
  const float size = fontSize > 0.0f ? fontSize : currentFontSize;
  queueText(text, fontPtr(), color, size, TextTransform(position));
}

bool Gui::button(StringView text, const Rect &rect) {
  const std::uint64_t id = hashId(text, rect);
  const bool hovered = contains(rect, mousePosition2D());
  const bool clicked = updateButtonState(id, hovered);
  queueRect(rect, widgetColor(id, hovered));

  auto *f = font();
  if (f != nullptr && f->isLoaded()) {
    const float fontSize = fitFontSize(text, rect, currentFontSize);
    queueText(text, fontPtr(), theme.text, fontSize,
              TextTransform(rect.center(), 0.0f, Pivot::Center));
  }
  return clicked;
}

bool Gui::checkbox(StringView text, bool checked, const Rect &rect) {
  const std::uint64_t id = hashId(text, rect);
  const bool hovered = contains(rect, mousePosition2D());
  const bool clicked = updateButtonState(id, hovered);
  const bool value = clicked ? !checked : checked;

  const float side = std::min(rect.width, rect.height);
  const Rect box(rect.x, rect.y + (rect.height - side) * 0.5f, side, side);
  queueRect(box, widgetColor(id, hovered));
  if (value) {
    const float pad = std::max(3.0f, side * 0.22f);
    queueRect(Rect(box.x + pad, box.y + pad, box.width - pad * 2.0f,
                   box.height - pad * 2.0f),
              theme.accent, 1);
  }

  auto *f = font();
  if (f != nullptr && f->isLoaded()) {
    const Vec2 textPos(rect.x + side + 8.0f, rect.center().y);
    queueText(text, fontPtr(), theme.text, currentFontSize,
              TextTransform(textPos, 0.0f, Pivot::Left));
  }
  return value;
}

float Gui::sliderFloat(StringView text, float value, float min, float max,
                       const Rect &rect) {
  if (max < min) {
    std::swap(min, max);
  }
  const std::uint64_t id = hashId(text, rect);
  const Vec2 mouse = mousePosition2D();
  const bool hovered = contains(rect, mouse);
  if (hovered) {
    hotId = id;
  }
  if (hovered && Mouse::isPressed(Mouse::LEFT)) {
    activeId = id;
  }
  if (activeId == id && Mouse::isDown(Mouse::LEFT) && max > min) {
    const float t = std::clamp((mouse.x - rect.x) / rect.width, 0.0f, 1.0f);
    value = min + (max - min) * t;
  }
  if (activeId == id && Mouse::isReleased(Mouse::LEFT)) {
    activeId = 0;
  }
  value = std::clamp(value, min, max);

  queueRect(rect, widgetColor(id, hovered));
  const float t = max > min ? (value - min) / (max - min) : 0.0f;
  queueRect(Rect(rect.x, rect.y, rect.width * t, rect.height), theme.accent,
            1);

  auto *f = font();
  if (f != nullptr && f->isLoaded()) {
    String label(text);
    label += ": ";
    label += std::format("{:.2f}", value);
    queueText(label, fontPtr(), theme.text, currentFontSize,
              TextTransform(Vec2(rect.x + 8.0f, rect.center().y), 0.0f,
                            Pivot::Left));
  }
  return value;
}

float Gui::scrollVertical(float scroll, const Rect &viewport,
                          float contentHeight, float wheelStep) {
  if (viewport.height <= 0.0f || contentHeight <= 0.0f) {
    return 0.0f;
  }

  const float maxScroll = std::max(0.0f, contentHeight - viewport.height);
  if (maxScroll <= 0.0f) {
    return 0.0f;
  }

  const std::uint64_t id = hashId("scrollVertical", viewport);
  const Rect track(viewport.x + viewport.width - 8.0f, viewport.y, 8.0f,
                   viewport.height);
  const Vec2 mouse = mousePosition2D();
  const bool hoveredViewport = contains(viewport, mouse);
  const bool hoveredTrack = contains(track, mouse);
  const Vec2 wheel = Mouse::getScrollWheel();

  if (hoveredViewport && wheel.y != 0.0f) {
    scroll -= wheel.y * wheelStep;
  }

  const float thumbHeight =
      std::max(24.0f, viewport.height * (viewport.height / contentHeight));
  const float thumbRange = std::max(1.0f, track.height - thumbHeight);
  auto thumbY = [&]() {
    return track.y +
           (std::clamp(scroll, 0.0f, maxScroll) / maxScroll) * thumbRange;
  };
  Rect thumb(track.x, thumbY(), track.width, thumbHeight);

  if (hoveredTrack && Mouse::isPressed(Mouse::LEFT)) {
    activeId = id;
  }
  if (activeId == id && Mouse::isDown(Mouse::LEFT)) {
    const float local =
        std::clamp(mouse.y - track.y - thumbHeight * 0.5f, 0.0f, thumbRange);
    scroll = (local / thumbRange) * maxScroll;
    thumb.y = thumbY();
  }
  if (activeId == id && Mouse::isReleased(Mouse::LEFT)) {
    activeId = 0;
  }

  scroll = std::clamp(scroll, 0.0f, maxScroll);
  thumb.y = thumbY();

  queueRect(track, Color(theme.background.r, theme.background.g,
                         theme.background.b, 0.55f));
  queueRect(thumb, activeId == id ? theme.active : theme.accent, 1);
  return scroll;
}
} // namespace sinen

namespace sinen {} // namespace sinen
