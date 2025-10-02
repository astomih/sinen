#include "core/io/asset_io.hpp"
#include "main_system.hpp"
#if ZEP_SINGLE_HEADER == 1
#define ZEP_SINGLE_HEADER_BUILD
#endif

#include "editor.hpp"

#include "asset/script/script_system.hpp"
#include "platform/input/input_system.hpp"
#include "platform/input/keyboard.hpp"
#include "platform/window/window_system.hpp"
#include <core/io/file_system.hpp>

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
  auto pImFont = ImGui::GetIO().Fonts->Fonts.front();
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
    strName = FileSystem::getAppBaseDirectory() + "/" +
              MainSystem::GetBasePath() + "/asset/script/main.lua";
    break;
  case ScriptType::Python:
    strName = FileSystem::getAppBaseDirectory() + "/" +
              MainSystem::GetBasePath() + "/asset/script/main.py";
    break;
  }
  auto pBuffer = zep_get_editor().InitWithFile(strName);
  assert(pBuffer != nullptr);
#ifdef __ANDROID__
  spZep->zepEditor.SetGlobalMode(Zep::ZepMode_Standard::StaticName());
#else
  spZep->zepEditor.SetGlobalMode(Zep::ZepMode_Vim::StaticName());
#endif
}

namespace fs = std::filesystem;

void showLuaScriptsMenu_While(const std::string &rootDir,
                              decltype(spZep) spZep) {
  if (!fs::exists(rootDir))
    return;

  std::stack<fs::path> dirs;
  dirs.push(rootDir);

  while (!dirs.empty()) {
    fs::path current = dirs.top();
    dirs.pop();

    if (current.filename() == "script" || current.filename() == "shader" ||
        current.parent_path().filename() == "script" ||
        current.parent_path().filename() == "shader") {
      std::vector<fs::directory_entry> files;
      for (auto &entry : fs::directory_iterator(current)) {
        if (entry.is_directory())
          continue;
        if (entry.path().extension() == ".slang") {
          files.push_back(entry);
          continue;
        }
        switch (sinen::ScriptSystem::GetType()) {
        case sinen::ScriptType::Lua:
          if (entry.path().extension() == ".lua")
            files.push_back(entry);
          break;
        case sinen::ScriptType::Python:

          if (entry.path().extension() == ".py")
            files.push_back(entry);
          break;
        default:
          break;
        }
      }
      if (files.empty())
        continue;

      std::sort(files.begin(), files.end(), [](auto &a, auto &b) {
        return a.path().filename().string() < b.path().filename().string();
      });

      fs::path parent = current.parent_path().parent_path();
      if (current.parent_path().filename() == "script" ||
          current.parent_path().filename() == "shader") {
        parent = current;
      }

      std::string menuLabel = parent.filename().string();

      if (parent == rootDir || menuLabel.empty()) {
        for (auto &f : files) {
          std::string label = f.path().filename().string();
          if (ImGui::MenuItem(label.c_str())) {
            spZep->zepEditor.InitWithFile(f.path().string());
          }
        }
      } else {
        if (ImGui::BeginMenu(menuLabel.c_str())) {
          for (auto &f : files) {
            std::string label = f.path().filename().string();
            if (ImGui::MenuItem(label.c_str())) {
              spZep->zepEditor.InitWithFile(f.path().string());
            }
          }
          ImGui::EndMenu();
        }
      }
    } else {
      std::vector<fs::directory_entry> files;
      for (auto &entry : fs::directory_iterator(current)) {
        if (entry.is_regular_file()) {
          if (entry.path().extension() == ".json")
            files.push_back(entry);
        }
      }
      if (!files.empty()) {
        std::string menuLabel = current.filename().string();

        if (current == rootDir || menuLabel.empty()) {
          for (auto &f : files) {
            std::string label = f.path().filename().string();
            if (ImGui::MenuItem(label.c_str())) {
              spZep->zepEditor.InitWithFile(f.path().string());
            }
          }
        } else {
          if (ImGui::BeginMenu(menuLabel.c_str())) {
            for (auto &f : files) {
              std::string label = f.path().filename().string();
              if (ImGui::MenuItem(label.c_str())) {
                spZep->zepEditor.InitWithFile(f.path().string());
              }
            }
            ImGui::EndMenu();
          }
        }
      }
    }

    for (auto &entry : fs::directory_iterator(current)) {
      if (entry.is_directory()) {
        dirs.push(entry.path());
      }
    }
  }
}

void zep_show(const Zep::NVec2i &displaySize) {
  bool show = true;
  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(displaySize.x, displaySize.y),
                           ImGuiCond_Always);
  static bool preWindowFocus = false;
  if (!ImGui::Begin("Editor", nullptr,
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar |
                        ImGuiWindowFlags_ChildMenu)) {
    ImGui::End();
    return;
  }
  if (ImGui::IsWindowFocused()) {
    if (!preWindowFocus) {
      SDL_StartTextInput(WindowSystem::get_sdl_window());
    }
    preWindowFocus = true;
  } else {
    if (preWindowFocus) {
      SDL_StopTextInput(WindowSystem::get_sdl_window());
    }
    preWindowFocus = false;
  }

  if (ImGui::BeginMenuBar()) {

    if (ImGui::BeginMenu("File")) {

      if (ImGui::MenuItem("New", "Ctrl+N")) {
      }
      if (ImGui::BeginMenu("Load")) {
        showLuaScriptsMenu_While(FileSystem::getAppBaseDirectory(), spZep);
        ImGui::EndMenu();
      }
      if (ImGui::MenuItem("Save")) {
        auto &editor = spZep->zepEditor;
        auto &pBuffer =
            editor.GetActiveTabWindow()->GetActiveWindow()->GetBuffer();
        editor.SaveBuffer(pBuffer);
      }
      if (ImGui::MenuItem("Close")) {
        auto &editor = spZep->zepEditor;
        auto count = editor.GetTabWindows().size();
        editor.GetActiveTabWindow()->CloseActiveWindow();
      }
      ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Run")) {
      MainSystem::Change("main", MainSystem::GetBasePath());
    }
    ImGui::EndMenuBar();
  }

  ImGui::Text("FPS: %.3f", ImGui::GetIO().Framerate);

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
  bool zepFocused = ImGui::IsWindowFocused();
  if (zepFocused) {
    spZep->zepEditor.HandleInput();
  }

  // TODO: A Better solution for this; I think the audio graph is creating a
  // new window and stealing focus
  static int focus_count = 0;
  if (focus_count++ < 2) {
    ImGui::SetWindowFocus();
  }
  ImGui::End();
}
} // namespace sinen