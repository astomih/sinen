#include "core/io/asset_io.hpp"
#if ZEP_SINGLE_HEADER == 1
#define ZEP_SINGLE_HEADER_BUILD
#endif

#include "editor.hpp"

#include "../../logic/script/script_system.hpp"
#include "../../platform/input/input_system.hpp"
#include "platform/input/keyboard.hpp"

#include <filesystem>
#include <functional>

namespace sinen {

namespace fs = std::filesystem;

using namespace Zep;
struct ZepWrapper : public Zep::IZepComponent {
  ZepWrapper(const fs::path &root_path, const Zep::NVec2f &pixelScale,
             std::function<void(std::shared_ptr<Zep::ZepMessage>)> fnCommandCB)
      : zepEditor(fs::path(root_path.string()), pixelScale),
        Callback(fnCommandCB) {
    zepEditor.RegisterCallback(this);
  }

  virtual Zep::ZepEditor &GetEditor() const override {
    return (Zep::ZepEditor &)zepEditor;
  }

  virtual void Notify(std::shared_ptr<Zep::ZepMessage> message) override {
    Callback(message);

    return;
  }

  virtual void HandleInput() { zepEditor.HandleInput(); }

  Zep::ZepEditor_ImGui zepEditor;
  std::function<void(std::shared_ptr<Zep::ZepMessage>)> Callback;
};

std::shared_ptr<ZepWrapper> spZep;

void zep_init(const Zep::NVec2f &pixelScale) {
  // Initialize the editor and watch for changes
  spZep = std::make_shared<ZepWrapper>(
      ".", Zep::NVec2f(pixelScale.x, pixelScale.y),
      [](std::shared_ptr<ZepMessage> spMessage) -> void {});

  // This is an example of adding different fonts for text styles.
  // If you ":e test.md" in the editor and type "# Heading 1" you will
  // see that Zep picks a different font size for the heading.
  auto &editor = spZep->GetEditor();
  editor.GetConfig().style = EditorStyle::Minimal;
  auto &display = spZep->GetEditor().GetDisplay();
  auto pImFont = ImGui::GetIO().Fonts[0].Fonts[0];
  auto pixelHeight = pImFont->FontSize;
  display.SetFont(ZepTextType::UI, std::make_shared<ZepFont_ImGui>(
                                       display, pImFont, int(pixelHeight)));
  display.SetFont(ZepTextType::Text, std::make_shared<ZepFont_ImGui>(
                                         display, pImFont, int(pixelHeight)));
  display.SetFont(ZepTextType::Heading1,
                  std::make_shared<ZepFont_ImGui>(display, pImFont,
                                                  int(pixelHeight * 1.5)));
  display.SetFont(ZepTextType::Heading2,
                  std::make_shared<ZepFont_ImGui>(display, pImFont,
                                                  int(pixelHeight * 1.25)));
  display.SetFont(ZepTextType::Heading3,
                  std::make_shared<ZepFont_ImGui>(display, pImFont,
                                                  int(pixelHeight * 1.125)));
}

void zep_update() {
  // This is required to make the editor cursor blink, and for the
  // :ZTestFlash example
  if (spZep) {
    spZep->GetEditor().RefreshRequired();
  }
}

void zep_destroy() { spZep.reset(); }

ZepEditor &zep_get_editor() { return spZep->GetEditor(); }

void zep_load() {

  auto scriptType = sinen::ScriptSystem::GetType();
  std::string strName;
  switch (scriptType) {
  case sinen::ScriptType::Lua:
    strName = "asset/script/main.lua";
    break;
  }
  auto pBuffer = zep_get_editor().InitWithFile(strName);
  assert(pBuffer != nullptr);
}

void zep_show(const Zep::NVec2i &displaySize) {
  bool show = true;
  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(displaySize.x, displaySize.y),
                           ImGuiCond_Always);
  if (!ImGui::Begin("Script Editor", nullptr, ImGuiWindowFlags_NoResize)) {
    ImGui::End();
    return;
  }

  auto min = ImGui::GetCursorScreenPos();
  auto max = ImGui::GetContentRegionAvail();
  if (max.x <= 0)
    max.x = 1;
  if (max.y <= 0)
    max.y = 1;
  ImGui::InvisibleButton("ZepContainer", max);

  // Fill the window
  max.x = min.x + max.x;
  max.y = min.y + max.y;

  spZep->zepEditor.SetDisplayRegion(Zep::NVec2f(min.x, min.y),
                                    Zep::NVec2f(max.x, max.y));
  spZep->zepEditor.Display();
  bool zep_focused = ImGui::IsWindowFocused();
  if (zep_focused) {
    spZep->zepEditor.HandleInput();
  }

  // TODO: A Better solution for this; I think the audio graph is creating a new
  // window and stealing focus
  static int focus_count = 0;
  if (focus_count++ < 2) {
    ImGui::SetWindowFocus();
  }
  ImGui::End();
}
} // namespace sinen