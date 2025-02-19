#include "editor.hpp"
#include <SDL3/SDL.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_sdl3.h>
// Added for ImGui
#include <ImGuizmo.h>

#include "glsl_editor.hpp"
#include "log_window.hpp"
#include "markdown.hpp"
#include "texteditor.hpp"

#include <sinen/sinen.hpp>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <sol/sol.hpp>
#if defined(_WIN32)
#include <windows.h>
#endif
#include <filesystem>
namespace sinen {
std::vector<matrix4> editor::m_matrices;
int editor::index_actors = 0;
int editor::index_components = 0;
std::string editor::current_file_name = "";
class editor::implements {
public:
  bool is_run = false;
  bool is_save = false;
};
static bool is_save_as = false;
static char save_as_path[256] = "";
static bool request_pop_func = false;
void editor::inspector() {

  ImGui::Begin("Scene View");
  ImVec2 uv_s, uv_e;
  uv_s = ImVec2(0, 0);
  uv_e = ImVec2(1, 1);

  // ImGui::Image((ImTextureID)Renderer::get_texture_id(),
  //              ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()));
  ImGuizmo::Enable(true);
  // Set rect to Gizmo window
  ImGuizmo::SetRect(0, 0, Window::size().x, Window::size().y);
  ImGuizmo::BeginFrame();

  ImGui::End();
}
void editor::load_scene(const std::string &path) {}
void editor::save_scene(const std::string &path) {}
void editor::save_as_scene() {}
void editor::menu() {
  ImGui::Begin("Menu", nullptr,
               ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar |
                   ImGuiWindowFlags_NoBringToFrontOnFocus);
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::BeginMenu("Load")) {
        ImGui::BeginPopup("Load");
        std::filesystem::path p("./data/scene");
        for (auto &entry : std::filesystem::directory_iterator(p)) {
          if (ImGui::MenuItem(entry.path().filename().string().c_str())) {
            current_file_name = entry.path().filename().string();
            load_scene(current_file_name);
          }
        }
        ImGui::EndPopup();
      }
      if (ImGui::MenuItem("Save", "Ctrl+S")) {
        save_scene(current_file_name);
      }
      if (ImGui::MenuItem("Save As", "Ctrl+Shift+S")) {
        save_as_scene();
      }
      if (ImGui::MenuItem("Exit", "Alt+F4")) {
        ImGui::SaveIniSettingsToDisk("layout.ini");
#if !defined(EMSCRIPTEN)
        std::exit(0);
#else
        emscripten_force_exit(0);
#endif
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
      if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
      }
      if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {
      } // Disabled item
      ImGui::Separator();
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Window")) {
      if (ImGui::MenuItem("Log", "Ctrl+L")) {
      }
      if (ImGui::MenuItem("Inspector", "Ctrl+I")) {
      }
      if (ImGui::MenuItem("Text Editor", "Ctrl+E")) {
      }
      if (ImGui::MenuItem("Document", "Ctrl+D")) {
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Run")) {
      if (ImGui::MenuItem("Run", "F5")) {
        run();
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
  ImGui::End();
}

editor::editor() : m_impl(std::make_unique<editor::implements>()) {}
editor::~editor() {}
void editor::setup() {
  ImGui::LoadIniSettingsFromDisk("layout.ini");
  // Soft Cherry style by Patitotective from ImThemes
  ImGuiStyle &style = ImGui::GetStyle();
  style.ScaleAllSizes(1.25);

  style.Alpha = 1.0f;
  style.DisabledAlpha = 0.4000000059604645f;
  style.WindowPadding = ImVec2(10.0f, 10.0f);
  style.WindowRounding = 4.0f;
  style.WindowBorderSize = 0.0f;
  style.WindowMinSize = ImVec2(50.0f, 50.0f);
  style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
  style.WindowMenuButtonPosition = ImGuiDir_Left;
  style.ChildRounding = 0.0f;
  style.ChildBorderSize = 1.0f;
  style.PopupRounding = 1.0f;
  style.PopupBorderSize = 1.0f;
  style.FramePadding = ImVec2(5.0f, 3.0f);
  style.FrameRounding = 3.0f;
  style.FrameBorderSize = 0.0f;
  style.ItemSpacing = ImVec2(6.0f, 6.0f);
  style.ItemInnerSpacing = ImVec2(3.0f, 2.0f);
  style.CellPadding = ImVec2(3.0f, 3.0f);
  style.IndentSpacing = 6.0f;
  style.ColumnsMinSpacing = 6.0f;
  style.ScrollbarSize = 13.0f;
  style.ScrollbarRounding = 16.0f;
  style.GrabMinSize = 20.0f;
  style.GrabRounding = 4.0f;
  style.TabRounding = 4.0f;
  style.TabBorderSize = 1.0f;
  style.TabMinWidthForCloseButton = 0.0f;
  style.ColorButtonPosition = ImGuiDir_Right;
  style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
  style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

  style.Colors[ImGuiCol_Text] =
      ImVec4(0.8588235378265381f, 0.929411768913269f, 0.886274516582489f, 1.0f);
  style.Colors[ImGuiCol_TextDisabled] = ImVec4(
      0.5215686559677124f, 0.5490196347236633f, 0.5333333611488342f, 1.0f);
  style.Colors[ImGuiCol_WindowBg] = ImVec4(
      0.1294117718935013f, 0.1372549086809158f, 0.168627455830574f, 1.0f);
  style.Colors[ImGuiCol_ChildBg] = ImVec4(
      0.1490196138620377f, 0.1568627506494522f, 0.1882352977991104f, 1.0f);
  style.Colors[ImGuiCol_PopupBg] = ImVec4(
      0.2000000029802322f, 0.2196078449487686f, 0.2666666805744171f, 1.0f);
  style.Colors[ImGuiCol_Border] = ImVec4(
      0.1372549086809158f, 0.1137254908680916f, 0.1333333402872086f, 1.0f);
  style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
  style.Colors[ImGuiCol_FrameBg] = ImVec4(
      0.168627455830574f, 0.1843137294054031f, 0.2313725501298904f, 1.0f);
  style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(
      0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
  style.Colors[ImGuiCol_FrameBgActive] = ImVec4(
      0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
  style.Colors[ImGuiCol_TitleBg] = ImVec4(
      0.2313725501298904f, 0.2000000029802322f, 0.2705882489681244f, 1.0f);
  style.Colors[ImGuiCol_TitleBgActive] = ImVec4(
      0.501960813999176f, 0.07450980693101883f, 0.2549019753932953f, 1.0f);
  style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(
      0.2000000029802322f, 0.2196078449487686f, 0.2666666805744171f, 1.0f);
  style.Colors[ImGuiCol_MenuBarBg] = ImVec4(
      0.2000000029802322f, 0.2196078449487686f, 0.2666666805744171f, 1.0f);
  style.Colors[ImGuiCol_ScrollbarBg] =
      ImVec4(0.239215686917305f, 0.239215686917305f, 0.2196078449487686f, 1.0f);
  style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(
      0.3882353007793427f, 0.3882353007793427f, 0.3725490272045135f, 1.0f);
  style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(
      0.6941176652908325f, 0.6941176652908325f, 0.686274528503418f, 1.0f);
  style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(
      0.6941176652908325f, 0.6941176652908325f, 0.686274528503418f, 1.0f);
  style.Colors[ImGuiCol_CheckMark] = ImVec4(
      0.658823549747467f, 0.1372549086809158f, 0.1764705926179886f, 1.0f);
  style.Colors[ImGuiCol_SliderGrab] = ImVec4(
      0.6509804129600525f, 0.1490196138620377f, 0.3450980484485626f, 1.0f);
  style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(
      0.7098039388656616f, 0.2196078449487686f, 0.2666666805744171f, 1.0f);
  style.Colors[ImGuiCol_Button] = ImVec4(
      0.6509804129600525f, 0.1490196138620377f, 0.3450980484485626f, 1.0f);
  style.Colors[ImGuiCol_ButtonHovered] = ImVec4(
      0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
  style.Colors[ImGuiCol_ButtonActive] = ImVec4(
      0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
  style.Colors[ImGuiCol_Header] = ImVec4(
      0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
  style.Colors[ImGuiCol_HeaderHovered] = ImVec4(
      0.6509804129600525f, 0.1490196138620377f, 0.3450980484485626f, 1.0f);
  style.Colors[ImGuiCol_HeaderActive] = ImVec4(
      0.501960813999176f, 0.07450980693101883f, 0.2549019753932953f, 1.0f);
  style.Colors[ImGuiCol_Separator] = ImVec4(
      0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 1.0f);
  style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(
      0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 1.0f);
  style.Colors[ImGuiCol_SeparatorActive] = ImVec4(
      0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 1.0f);
  style.Colors[ImGuiCol_ResizeGrip] = ImVec4(
      0.6509804129600525f, 0.1490196138620377f, 0.3450980484485626f, 1.0f);
  style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(
      0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
  style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(
      0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
  style.Colors[ImGuiCol_Tab] = ImVec4(0.1764705926179886f, 0.3490196168422699f,
                                      0.5764706134796143f, 1.0f);
  style.Colors[ImGuiCol_TabHovered] = ImVec4(
      0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
  style.Colors[ImGuiCol_TabActive] =
      ImVec4(0.196078434586525f, 0.407843142747879f, 0.6784313917160034f, 1.0f);
  style.Colors[ImGuiCol_TabUnfocused] = ImVec4(
      0.06666667014360428f, 0.1019607856869698f, 0.1450980454683304f, 1.0f);
  style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(
      0.1333333402872086f, 0.2588235437870026f, 0.4235294163227081f, 1.0f);
  style.Colors[ImGuiCol_PlotLines] =
      ImVec4(0.8588235378265381f, 0.929411768913269f, 0.886274516582489f, 1.0f);
  style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(
      0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
  style.Colors[ImGuiCol_PlotHistogram] = ImVec4(
      0.3098039329051971f, 0.7764706015586853f, 0.196078434586525f, 1.0f);
  style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(
      0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
  style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(
      0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
  style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(
      0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
  style.Colors[ImGuiCol_TableBorderLight] = ImVec4(
      0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
  style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
  style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
  style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(
      0.3843137323856354f, 0.6274510025978088f, 0.9176470637321472f, 1.0f);
  style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
  style.Colors[ImGuiCol_NavHighlight] = ImVec4(
      0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
  style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
  style.Colors[ImGuiCol_NavWindowingDimBg] =
      ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 1.0f);
  style.Colors[ImGuiCol_ModalWindowDimBg] =
      ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f,
             0.300000011920929f);
  m_impl->is_run = false;
  Renderer::add_imgui_function(inspector);
  Renderer::add_imgui_function(menu);
  Renderer::add_imgui_function(markdown);
  Renderer::add_imgui_function(log_window);
  Renderer::add_imgui_function(texteditor::display);
  Renderer::add_imgui_function(glsl_editor::display);
  Renderer::toggle_show_imgui();
}
void editor::update(float delta_time) {
  if (request_pop_func) {
    Renderer::get_imgui_function().pop_back();
    request_pop_func = false;
  }

  if (Renderer::is_show_imgui() && Keyboard::is_pressed(Keyboard::code::F5)) {
    m_impl->is_run = true;
  }
  if (m_impl->is_run) {
    m_impl->is_run = false;
  }
  if (Renderer::is_show_imgui() && Keyboard::is_down(Keyboard::code::LCTRL) &&
      Keyboard::is_pressed(Keyboard::code::S)) {
    // m_impl->is_save = true;
  }
  if (m_impl->is_save) {
    auto str = texteditor::get_text();
    DataStream::write(AssetType::Script, Scene::current_name() + ".lua", str);
    std::cout << str << std::endl;
    m_impl->is_save = false;
  }
  if (Renderer::is_show_imgui() && Keyboard::is_down(Keyboard::code::LCTRL) &&
      Keyboard::is_pressed(Keyboard::code::S)) {
    if (Keyboard::is_down(Keyboard::code::LSHIFT)) {

    } else if (!current_file_name.empty()) {
      save_scene(this->current_file_name);
    }
  }
  if (Keyboard::is_pressed(Keyboard::code::F3)) {
    Renderer::toggle_show_imgui();
  }
  if (Keyboard::is_pressed(Keyboard::code::F5)) {
    run();
  }

  // Camera moved by mouse
  {
    // Normalize camera angle vector
    auto vec = Scene::main_camera().target() - Scene::main_camera().position();
    if (vec.length() > 0.1) {
      vec.normalize();
      Scene::main_camera().position() += vec * Mouse::get_scroll_wheel().y;
    }
    static Vector2 prev = Vector2();
    if (Mouse::is_down(Mouse::code::RIGHT)) {
      auto pos = prev - Mouse::get_position();
      if (Keyboard::is_down(Keyboard::code::LSHIFT)) {
        Scene::main_camera().position() +=
            Scene::main_camera().view().get_x_axis() * pos.x * delta_time;
        Scene::main_camera().target() +=
            Scene::main_camera().view().get_x_axis() * pos.x * delta_time;
        Scene::main_camera().position() -=
            Scene::main_camera().view().get_z_axis() * pos.y * delta_time;
        Scene::main_camera().target() -=
            Scene::main_camera().view().get_z_axis() * pos.y * delta_time;

      } else {
        Scene::main_camera().target().x += pos.x * delta_time;
        Scene::main_camera().target().y -= pos.y * delta_time;
      }
    }
    prev = Mouse::get_position();
  }
  Scene::main_camera().lookat(Scene::main_camera().position(),
                              Scene::main_camera().target(),
                              Scene::main_camera().up());
}
void editor::run() {
#ifdef _WIN32
  if (!current_file_name.empty()) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    std::string commandlp = std::string(std::string("Debug/Editor.exe ") +
                                        std::string(current_file_name));
    // Start the child process.
    {
      auto result = CreateProcess(NULL, (LPSTR)commandlp.c_str(), NULL, NULL,
                                  FALSE, 0, NULL, NULL, &si, &pi);
      if (result == 0) {
        Logger::error("Failed to run the application.");
      }
    }
  } else {
    Logger::error("The scene has not yet loaded anything.");
  }

#endif // _WIN32
}
} // namespace sinen
