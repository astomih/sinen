#include "editor.hpp"
#include <SDL.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_sdl.h>
// Added for ImGui
#include <ImGuizmo.h>

#include "glsl_editor.hpp"
#include "log_window.hpp"
#include "markdown.hpp"
#include "texteditor.hpp"

#include <sinen/sinen.hpp>

#include <component/draw3d_component.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <sol/sol.hpp>
#if defined(_WIN32)
#include <windows.h>
#endif
#include <filesystem>
namespace sinen {
std::vector<Actor> editor::m_actors;
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

  ImGui::Image((void *)Renderer::get_texture_id(),
               ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()));
  ImGuizmo::Enable(true);
  // Set rect to Gizmo window
  ImGuizmo::SetRect(0, 0,
                    Window::size().x, Window::size().y);
  ImGuizmo::BeginFrame();

  ImGui::End();
  ImGui::Begin("Inspector");
  static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
  static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
  if (m_matrices.size() > 0) {
    ImGuizmo::Manipulate(Scene::main_camera().view().get(),
                         Scene::main_camera().projection().get(),
                         mCurrentGizmoOperation, mCurrentGizmoMode,
                         m_matrices[index_actors].mat.m16);
  }
  if (Keyboard::is_pressed(Keyboard::code::T))
    mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
  if (Keyboard::is_pressed(Keyboard::code::R))
    mCurrentGizmoOperation = ImGuizmo::ROTATE;
  if (Keyboard::is_pressed(Keyboard::code::S))
    mCurrentGizmoOperation = ImGuizmo::SCALE;
  if (ImGui::RadioButton("Translate",
                         mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
    mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
  ImGui::SameLine();
  if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
    mCurrentGizmoOperation = ImGuizmo::ROTATE;
  ImGui::SameLine();
  if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
    mCurrentGizmoOperation = ImGuizmo::SCALE;
  ImGuizmo::AllowAxisFlip(false);
  ImGui::Text("Transform");
  Vector3 pos, rot, scale;
  if (m_actors.size() > 0) {
    ImGuizmo::DecomposeMatrixToComponents(m_matrices[index_actors].mat.m16,
                                          &pos.x, &rot.x, &scale.x);
    ImGui::DragFloat3("Position", &pos.x);
    ImGui::DragFloat3("Rotation", &rot.x);
    ImGui::DragFloat3("Scale", &scale.x);
    ImGuizmo::RecomposeMatrixFromComponents(&pos.x, &rot.x, &scale.x,
                                            m_matrices[index_actors].mat.m16);
    m_actors[index_actors].set_position(pos);
    m_actors[index_actors].set_rotation(rot);
    m_actors[index_actors].set_scale(scale);
  } else {
    float dummy[3] = {0, 0, 0};
    ImGui::DragFloat3("Position", dummy);
    ImGui::DragFloat3("Rotation", dummy);
    ImGui::DragFloat3("Scale", dummy);
  }
  ImGui::Separator();

  static std::string buf = "";
  if (ImGui::CollapsingHeader("Actors", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::Button("AddActor")) {
      m_actors.push_back(Actor{});
      auto m = matrix4::identity;
      m_matrices.push_back(m);
      if (m_actors.size() == 1) {
        buf = m_actors[index_actors].get_script_name();
      }
      m_actors.back().set_name("Actor" + std::to_string(m_actors.size()));
    }
    ImGui::SameLine();
    if (ImGui::Button("RemoveActor")) {
      if (!m_actors.empty()) {
        m_actors.erase(m_actors.begin() + index_actors);
        m_matrices.erase(m_matrices.begin() + index_actors);
        if (index_actors > 0) {
          index_actors--;
        }
      }
    }
    if (!m_actors.empty()) {
      static char name[128] = {};
      ImGui::InputText("Actor", name, sizeof(name));
      if (ImGui::Button("Set Name")) {
        m_actors[index_actors].set_name(name);
      }
      ImGui::InputText("Script", buf.data(), buf.size());
      if (ImGui::Button("Open")) {
        if (!m_actors.empty()) {
          texteditor::set_script_name(buf);
          texteditor::set_text(
              DataStream::open_as_string(AssetType::Script, buf));
          Script::do_script(buf);
        }
      }
    }
    for (int i = 0; i < m_actors.size(); i++) {
      if (ImGui::RadioButton(m_actors[i].get_name().c_str(), &index_actors,
                             i)) {
        buf = m_actors[index_actors].get_script_name();
      }
    }
  }
  if (ImGui::CollapsingHeader("Components", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (m_actors.size() > 0) {
      std::vector<const char *> listbox_items;
      static int listbox_item_current = 0;
      auto comp_names = Scene::get_component_factory().get_component_names();
      for (auto &c : comp_names) {
        listbox_items.push_back(c.c_str());
      }

      ImGui::ListBox("Component list", &listbox_item_current,
                     listbox_items.data(), listbox_items.size(), 3);
      if (ImGui::Button("AddComponent")) {
        m_actors[index_actors].add_component(
            listbox_items[listbox_item_current]);
      }
      ImGui::SameLine();
      if (ImGui::Button("RemoveComponent")) {
        m_actors[index_actors].remove_component(
            listbox_items[listbox_item_current]);
      }
      for (auto &c : m_actors[index_actors].get_components()) {
        ImGui::Text("%s", c->get_name().c_str());
      }
    }
  }
  ImGui::End();
}
void editor::load_scene(const std::string &path) {
  index_actors = 0;
  auto str = DataStream::open_as_string(AssetType::Scene, path);
  Json j;
  j.parse(str);
  // Camera
  {
    auto camera_data = j["Camera"];
    Scene::main_camera().position() =
        Vector3(camera_data["Position"]["x"].get_float(),
                camera_data["Position"]["y"].get_float(),
                camera_data["Position"]["z"].get_float());
    Scene::main_camera().target() =
        Vector3(camera_data["Target"]["x"].get_float(),
                camera_data["Target"]["y"].get_float(),
                camera_data["Target"]["z"].get_float());
    Scene::main_camera().up() = Vector3(camera_data["Up"]["x"].get_float(),
                                        camera_data["Up"]["y"].get_float(),
                                        camera_data["Up"]["z"].get_float());
    Scene::main_camera().lookat(Scene::main_camera().position(),
                                Scene::main_camera().target(),
                                Scene::main_camera().up());
  }
  // Actors
  {
    m_actors.clear();
    m_matrices.clear();
    m_actors.resize(j["Actors"].get_array().size());
    m_matrices.resize(j["Actors"].get_array().size());
    Texture tex;
    tex.fill_color(Palette::white());
    for (int i = 0; i < m_actors.size(); i++) {
      auto act = j["Actors"].get_array()[i];
      m_actors[i].set_name(act["Name"].get_string());
      m_actors[i].set_script_name(act["Script"].get_string());
      m_actors[i].set_position(Vector3(act["Position"]["x"].get_float(),
                                       act["Position"]["y"].get_float(),
                                       act["Position"]["z"].get_float()));
      m_actors[i].set_rotation(Vector3(act["Rotation"]["x"].get_float(),
                                       act["Rotation"]["y"].get_float(),
                                       act["Rotation"]["z"].get_float()));
      m_actors[i].set_scale(Vector3(act["Scale"]["x"].get_float(),
                                    act["Scale"]["y"].get_float(),
                                    act["Scale"]["z"].get_float()));
      Vector3 pos, rot, scale;
      pos = m_actors[i].get_position();
      rot = m_actors[i].get_rotation();
      scale = m_actors[i].get_scale();
      ImGuizmo::RecomposeMatrixFromComponents(&pos.x, &rot.x, &scale.x,
                                              m_matrices[i].get());
      for (int j = 0; j < act["Components"].get_array().size(); j++) {
        auto comp = act["Components"].get_array()[j];
        auto comp_name = comp.get_string();
        auto c = Scene::get_component_factory().create(comp_name, m_actors[i]);
        m_actors[i].add_component(c);
      }
    }
  }
  Logger::info(
      std::string("Scene " + std::string(current_file_name) + " loaded"));
}
void editor::save_scene(const std::string &path) {
  std::string str;
  str = "{}";
  Json j;
  j.parse(str);
  auto camera_data = j.create_object();
  {
    {
      auto &pos = Scene::main_camera().position();
      auto position = j.create_object();
      position.add_member("x", pos.x);
      position.add_member("y", pos.y);
      position.add_member("z", pos.z);
      camera_data.add_member("Position", position);
    }
    {
      auto &target = Scene::main_camera().target();
      auto targetobj = j.create_object();
      targetobj.add_member("x", target.x);
      targetobj.add_member("y", target.y);
      targetobj.add_member("z", target.z);
      camera_data.add_member("Target", targetobj);
    }
    {
      auto &up = Scene::main_camera().up();
      auto upobj = j.create_object();
      upobj.add_member("x", up.x);
      upobj.add_member("y", up.y);
      upobj.add_member("z", up.z);
      camera_data.add_member("Up", upobj);
    }
  }
  j.add_member("Camera", camera_data);
  auto actors = j.create_array();
  {
    for (int i = 0; i < m_actors.size(); i++) {
      auto act = j.create_object();
      {
        { act.add_member("Name", m_actors[i].get_name()); }
        { act.add_member("Script", m_actors[i].get_script_name()); }
        {
          Vector3 pos = m_actors[i].get_position();
          auto position = j.create_object();
          position.add_member("x", pos.x);
          position.add_member("y", pos.y);
          position.add_member("z", pos.z);
          act.add_member("Position", position);
        }
        {
          Vector3 rot = m_actors[i].get_rotation();
          auto rotation = j.create_object();
          rotation.add_member("x", rot.x);
          rotation.add_member("y", rot.y);
          rotation.add_member("z", rot.z);
          act.add_member("Rotation", rotation);
        }
        {
          Vector3 scale = m_actors[i].get_scale();
          auto s = j.create_object();
          s.add_member("x", scale.x);
          s.add_member("y", scale.y);
          s.add_member("z", scale.z);
          act.add_member("Scale", s);
        }
        {
          auto arr = j.create_array();
          for (auto &c : m_actors[i].get_components()) {
            auto obj = j.create_object();
            auto str = c->get_name();
            obj.set_string(str);
            arr.push_back(obj);
          }
          act.add_member("Components", arr);
        }
      }
      actors.push_back(act);
    }
  }
  j.add_member("Actors", actors);
  auto s = j.to_string();
  File f;
  f.open("data/scene/" + path, File::mode::w);
  f.write(s.c_str(), s.size(), 1);
  f.close();
  Logger::info("Scene saved");
}
void editor::save_as_scene() {
  if (!is_save_as) {
    Renderer::add_imgui_function([&]() {
      ImGui::Begin("Input File Name");
      is_save_as = true;
      ImGui::InputText("File Name", save_as_path, 256);
      if (ImGui::Button("Save")) {
        current_file_name = save_as_path;
        File f;
        f.open("data/scene/" + current_file_name, File::mode::wp);
        f.close();
        save_scene(current_file_name);
        is_save_as = false;
        request_pop_func = true;
      }
      ImGui::End();
    });
  }
}
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
	ImGuiStyle& style = ImGui::GetStyle();
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
	
	style.Colors[ImGuiCol_Text] = ImVec4(0.8588235378265381f, 0.929411768913269f, 0.886274516582489f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5215686559677124f, 0.5490196347236633f, 0.5333333611488342f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1294117718935013f, 0.1372549086809158f, 0.168627455830574f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.1490196138620377f, 0.1568627506494522f, 0.1882352977991104f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2666666805744171f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.1372549086809158f, 0.1137254908680916f, 0.1333333402872086f, 1.0f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.168627455830574f, 0.1843137294054031f, 0.2313725501298904f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.2313725501298904f, 0.2000000029802322f, 0.2705882489681244f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.501960813999176f, 0.07450980693101883f, 0.2549019753932953f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2666666805744171f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2666666805744171f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.239215686917305f, 0.239215686917305f, 0.2196078449487686f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3882353007793427f, 0.3882353007793427f, 0.3725490272045135f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.6941176652908325f, 0.6941176652908325f, 0.686274528503418f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.6941176652908325f, 0.6941176652908325f, 0.686274528503418f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.658823549747467f, 0.1372549086809158f, 0.1764705926179886f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.6509804129600525f, 0.1490196138620377f, 0.3450980484485626f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.7098039388656616f, 0.2196078449487686f, 0.2666666805744171f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.6509804129600525f, 0.1490196138620377f, 0.3450980484485626f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.6509804129600525f, 0.1490196138620377f, 0.3450980484485626f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.501960813999176f, 0.07450980693101883f, 0.2549019753932953f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 1.0f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 1.0f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.6509804129600525f, 0.1490196138620377f, 0.3450980484485626f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.1764705926179886f, 0.3490196168422699f, 0.5764706134796143f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.196078434586525f, 0.407843142747879f, 0.6784313917160034f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667014360428f, 0.1019607856869698f, 0.1450980454683304f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1333333402872086f, 0.2588235437870026f, 0.4235294163227081f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.8588235378265381f, 0.929411768913269f, 0.886274516582489f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.3098039329051971f, 0.7764706015586853f, 0.196078434586525f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.4549019634723663f, 0.196078434586525f, 0.2980392277240753f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.3843137323856354f, 0.6274510025978088f, 0.9176470637321472f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 1.0f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.300000011920929f);
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
  for (auto &act : m_actors) {
    act.update(delta_time);
    sol::state &lua = *(sol::state *)Script::get_state();
    lua.do_string(
        DataStream::open_as_string(AssetType::Script, act.get_script_name()));
    lua["update"]();
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
    std::string commandlp =
        std::string(std::string("app.exe ") + std::string(current_file_name));
    // Start the child process.
    {
      WINBOOL result = CreateProcess(NULL, (LPSTR)commandlp.c_str(), NULL, NULL,
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
