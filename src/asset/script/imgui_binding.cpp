#include <core/data/string.hpp>
#include <math/vector.hpp>
#include <sol/sol.hpp>

#include <imgui.h>

namespace sinen {

void bindImGui(sol::table &lua) {

  auto imgui = lua.create_named("ImGui");
  imgui["Begin"] =
      sol::overload([](StringView name) { return ImGui::Begin(name.data()); },
                    [](StringView name, int flags) {
                      ImGui::Begin(name.data(), nullptr, flags);
                    });
  imgui["End"] = &ImGui::End;
  imgui["button"] = [](StringView name) { return ImGui::Button(name.data()); };
  imgui["text"] = [](StringView text) { ImGui::Text("%s", text.data()); };
  imgui["setNextWindowPos"] = [](const Vec2 &pos) {
    ImGui::SetNextWindowPos({pos.x, pos.y});
  };
  imgui["setNextWindowSize"] = [](const Vec2 &size) {
    ImGui::SetNextWindowSize({size.x, size.y});
  };
  {
    auto windowFlags = imgui.create_named("WindowFlags");
    windowFlags["None"] = ImGuiWindowFlags_None;
    windowFlags["NoTitleBar"] = ImGuiWindowFlags_NoTitleBar;
    windowFlags["NoResize"] = ImGuiWindowFlags_NoResize;
    windowFlags["NoMove"] = ImGuiWindowFlags_NoMove;
    windowFlags["NoScrollBar"] = ImGuiWindowFlags_NoScrollbar;
    windowFlags["NoScrollWithMouse"] = ImGuiWindowFlags_NoScrollWithMouse;
    windowFlags["NoCollapse"] = ImGuiWindowFlags_NoCollapse;
    windowFlags["AlwaysAutoResize"] = ImGuiWindowFlags_AlwaysAutoResize;
    windowFlags["NoBackground"] = ImGuiWindowFlags_NoBackground;
    windowFlags["NoSavedSettings;"] = ImGuiWindowFlags_NoSavedSettings;
    windowFlags["NoMouseInputs"] = ImGuiWindowFlags_NoMouseInputs;
    windowFlags["MenuBar"] = ImGuiWindowFlags_MenuBar;
    windowFlags["HorizontalScrollbar"] = ImGuiWindowFlags_HorizontalScrollbar;
    windowFlags["NoFocusOnAppearing"] = ImGuiWindowFlags_NoFocusOnAppearing;
    windowFlags["NoBringToFrontOnFocus"] =
        ImGuiWindowFlags_NoBringToFrontOnFocus;
    windowFlags["AlwaysVerticalScrollbar"] =
        ImGuiWindowFlags_AlwaysVerticalScrollbar;
    windowFlags["AlwaysHorizontalScrollbar"] =
        ImGuiWindowFlags_AlwaysHorizontalScrollbar;
    windowFlags["NoNavInputs"] = ImGuiWindowFlags_NoNavInputs;
    windowFlags["NoNavFocus"] = ImGuiWindowFlags_NoNavFocus;
    windowFlags["UnsavedDocument"] = ImGuiWindowFlags_UnsavedDocument;
    windowFlags["NoNav"] = ImGuiWindowFlags_NoNav;
    windowFlags["NoDecoration"] = ImGuiWindowFlags_NoDecoration;
    windowFlags["NoInputs"] = ImGuiWindowFlags_NoInputs;
  }
}
} // namespace sinen