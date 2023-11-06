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
std::vector<actor> editor::m_actors;
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

  ImGui::Image((void *)renderer::get_texture_id(),
               ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - 20));
  ImGui::End();
  ImGuizmo::BeginFrame();
  ImGuizmo::Enable(true);
  ImGuizmo::SetRect(0, 0, window::size().x, window::size().y);
  ImGui::Begin("Inspector");
  static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
  static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
  if (keyboard::is_pressed(keyboard::code::T))
    mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
  if (keyboard::is_pressed(keyboard::code::R))
    mCurrentGizmoOperation = ImGuizmo::ROTATE;
  if (keyboard::is_pressed(keyboard::code::S))
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
  if (m_matrices.size() > 0) {
    ImGuizmo::Manipulate(scene::main_camera().view().get(),
                         scene::main_camera().projection().get(),
                         mCurrentGizmoOperation, mCurrentGizmoMode,
                         m_matrices[index_actors].mat.m16);
  }
  ImGui::Text("Transform");
  vector3 pos, rot, scale;
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
      m_actors.push_back(actor{});
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
              data_stream::open_as_string(asset_type::Script, buf));
          script::do_script(buf);
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
      auto comp_names = scene::get_component_factory().get_component_names();
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
  auto str = data_stream::open_as_string(asset_type::Scene, path);
  json j;
  j.parse(str);
  // Camera
  {
    auto camera_data = j["Camera"];
    scene::main_camera().position() =
        vector3(camera_data["Position"]["x"].get_float(),
                camera_data["Position"]["y"].get_float(),
                camera_data["Position"]["z"].get_float());
    scene::main_camera().target() =
        vector3(camera_data["Target"]["x"].get_float(),
                camera_data["Target"]["y"].get_float(),
                camera_data["Target"]["z"].get_float());
    scene::main_camera().up() = vector3(camera_data["Up"]["x"].get_float(),
                                        camera_data["Up"]["y"].get_float(),
                                        camera_data["Up"]["z"].get_float());
    scene::main_camera().lookat(scene::main_camera().position(),
                                scene::main_camera().target(),
                                scene::main_camera().up());
  }
  // Actors
  {
    m_actors.clear();
    m_matrices.clear();
    m_actors.resize(j["Actors"].get_array().size());
    m_matrices.resize(j["Actors"].get_array().size());
    texture tex;
    tex.fill_color(palette::white());
    for (int i = 0; i < m_actors.size(); i++) {
      auto act = j["Actors"].get_array()[i];
      m_actors[i].set_name(act["Name"].get_string());
      m_actors[i].set_script_name(act["Script"].get_string());
      m_actors[i].set_position(vector3(act["Position"]["x"].get_float(),
                                       act["Position"]["y"].get_float(),
                                       act["Position"]["z"].get_float()));
      m_actors[i].set_rotation(vector3(act["Rotation"]["x"].get_float(),
                                       act["Rotation"]["y"].get_float(),
                                       act["Rotation"]["z"].get_float()));
      m_actors[i].set_scale(vector3(act["Scale"]["x"].get_float(),
                                    act["Scale"]["y"].get_float(),
                                    act["Scale"]["z"].get_float()));
      vector3 pos, rot, scale;
      pos = m_actors[i].get_position();
      rot = m_actors[i].get_rotation();
      scale = m_actors[i].get_scale();
      ImGuizmo::RecomposeMatrixFromComponents(&pos.x, &rot.x, &scale.x,
                                              m_matrices[i].get());
      for (int j = 0; j < act["Components"].get_array().size(); j++) {
        auto comp = act["Components"].get_array()[j];
        auto comp_name = comp.get_string();
        auto c = scene::get_component_factory().create(comp_name, m_actors[i]);
        m_actors[i].add_component(c);
      }
    }
  }
  logger::info(
      std::string("Scene " + std::string(current_file_name) + " loaded"));
}
void editor::save_scene(const std::string &path) {
  std::string str;
  str = "{}";
  json j;
  j.parse(str);
  auto camera_data = j.create_object();
  {
    {
      auto &pos = scene::main_camera().position();
      auto position = j.create_object();
      position.add_member("x", pos.x);
      position.add_member("y", pos.y);
      position.add_member("z", pos.z);
      camera_data.add_member("Position", position);
    }
    {
      auto &target = scene::main_camera().target();
      auto targetobj = j.create_object();
      targetobj.add_member("x", target.x);
      targetobj.add_member("y", target.y);
      targetobj.add_member("z", target.z);
      camera_data.add_member("Target", targetobj);
    }
    {
      auto &up = scene::main_camera().up();
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
          vector3 pos = m_actors[i].get_position();
          auto position = j.create_object();
          position.add_member("x", pos.x);
          position.add_member("y", pos.y);
          position.add_member("z", pos.z);
          act.add_member("Position", position);
        }
        {
          vector3 rot = m_actors[i].get_rotation();
          auto rotation = j.create_object();
          rotation.add_member("x", rot.x);
          rotation.add_member("y", rot.y);
          rotation.add_member("z", rot.z);
          act.add_member("Rotation", rotation);
        }
        {
          vector3 scale = m_actors[i].get_scale();
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
  file f;
  f.open("data/scene/" + path, file::mode::w);
  f.write(s.c_str(), s.size(), 1);
  f.close();
  logger::info("Scene saved");
}
void editor::save_as_scene() {
  if (!is_save_as) {
    renderer::add_imgui_function([&]() {
      ImGui::Begin("Input File Name");
      is_save_as = true;
      ImGui::InputText("File Name", save_as_path, 256);
      if (ImGui::Button("Save")) {
        current_file_name = save_as_path;
        file f;
        f.open("data/scene/" + current_file_name, file::mode::wp);
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
  ImGui::SetNextWindowPos({0, 2});
  ImGui::SetNextWindowSize({window::size().x, 20});
  ImGui::Begin("Menu", nullptr,
               ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus |
                   ImGuiWindowFlags_NoTitleBar |
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
  ImGuiStyle &style = ImGui::GetStyle();
  style.Alpha = 1.0;
  style.ChildRounding = 3;
  style.WindowRounding = 3;
  style.GrabRounding = 1;
  style.GrabMinSize = 20;
  style.FrameRounding = 3;

  style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
  style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.00f, 0.40f, 0.41f, 1.00f);
  style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 1.00f, 1.00f, 0.65f);
  style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  style.Colors[ImGuiCol_FrameBg] = ImVec4(0.44f, 0.80f, 0.80f, 0.18f);
  style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.44f, 0.80f, 0.80f, 0.27f);
  style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.44f, 0.81f, 0.86f, 0.66f);
  style.Colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.18f, 0.21f, 0.73f);
  style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
  style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.27f);
  style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
  style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.22f, 0.29f, 0.30f, 0.71f);
  style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.00f, 1.00f, 1.00f, 0.44f);
  style.Colors[ImGuiCol_ScrollbarGrabHovered] =
      ImVec4(0.00f, 1.00f, 1.00f, 0.74f);
  style.Colors[ImGuiCol_ScrollbarGrabActive] =
      ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
  style.Colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 1.00f, 1.00f, 0.68f);
  style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 1.00f, 1.00f, 0.36f);
  style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.76f);
  style.Colors[ImGuiCol_Button] = ImVec4(0.00f, 0.65f, 0.65f, 0.46f);
  style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.01f, 1.00f, 1.00f, 0.43f);
  style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.62f);
  style.Colors[ImGuiCol_Header] = ImVec4(0.00f, 1.00f, 1.00f, 0.33f);
  style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 1.00f, 1.00f, 0.42f);
  style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.54f);
  style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 1.00f, 1.00f, 0.54f);
  style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 1.00f, 1.00f, 0.74f);
  style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
  style.Colors[ImGuiCol_PlotLines] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
  style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogramHovered] =
      ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
  style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 1.00f, 1.00f, 0.22f);
  style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.04f, 0.10f, 0.09f, 0.51f);
  m_impl->is_run = false;
  renderer::add_imgui_function(menu);
  renderer::add_imgui_function(markdown);
  renderer::add_imgui_function(log_window);
  renderer::add_imgui_function(inspector);
  renderer::add_imgui_function(texteditor::display);
  renderer::add_imgui_function(glsl_editor::display);
  renderer::toggle_show_imgui();
}
void editor::update(float delta_time) {
  if (request_pop_func) {
    renderer::get_imgui_function().pop_back();
    request_pop_func = false;
  }
  for (auto &act : m_actors) {
    act.update(delta_time);
    sol::state &lua = *(sol::state *)script::get_state();
    lua.do_string(
        data_stream::open_as_string(asset_type::Script, act.get_script_name()));
    lua["update"]();
  }

  if (renderer::is_show_imgui() && keyboard::is_pressed(keyboard::code::F5)) {
    m_impl->is_run = true;
  }
  if (m_impl->is_run) {
    m_impl->is_run = false;
  }
  if (renderer::is_show_imgui() && keyboard::is_down(keyboard::code::LCTRL) &&
      keyboard::is_pressed(keyboard::code::S)) {
    // m_impl->is_save = true;
  }
  if (m_impl->is_save) {
    auto str = texteditor::get_text();
    data_stream::write(asset_type::Script, scene::current_name() + ".lua", str);
    std::cout << str << std::endl;
    m_impl->is_save = false;
  }
  if (renderer::is_show_imgui() && keyboard::is_down(keyboard::code::LCTRL) &&
      keyboard::is_pressed(keyboard::code::S)) {
    if (keyboard::is_down(keyboard::code::LSHIFT)) {

    } else if (!current_file_name.empty()) {
      save_scene(this->current_file_name);
    }
  }
  if (keyboard::is_pressed(keyboard::code::F3)) {
    renderer::toggle_show_imgui();
  }
  if (keyboard::is_pressed(keyboard::code::F5)) {
    run();
  }

  // Camera moved by mouse
  {
    // Normalize camera angle vector
    auto vec = scene::main_camera().target() - scene::main_camera().position();
    if (vec.length() > 0.1) {
      vec.normalize();
      scene::main_camera().position() += vec * mouse::get_scroll_wheel().y;
    }
    static vector2 prev = vector2();
    if (mouse::is_down(mouse::code::RIGHT)) {
      auto pos = prev - mouse::get_position();
      if (keyboard::is_down(keyboard::code::LSHIFT)) {
        scene::main_camera().position() +=
            scene::main_camera().view().get_x_axis() * pos.x * delta_time;
        scene::main_camera().target() +=
            scene::main_camera().view().get_x_axis() * pos.x * delta_time;
        scene::main_camera().position() -=
            scene::main_camera().view().get_z_axis() * pos.y * delta_time;
        scene::main_camera().target() -=
            scene::main_camera().view().get_z_axis() * pos.y * delta_time;

      } else {
        scene::main_camera().target().x += pos.x * delta_time;
        scene::main_camera().target().y -= pos.y * delta_time;
      }
    }
    prev = mouse::get_position();
  }
  scene::main_camera().lookat(scene::main_camera().position(),
                              scene::main_camera().target(),
                              scene::main_camera().up());
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
        std::string(std::string("game.exe ") + std::string(current_file_name));
    // Start the child process.
    {
      WINBOOL result =
          CreateProcess(NULL, // No module name (use command line)
                        (LPSTR)commandlp.c_str(), // Command line
                        NULL,  // Process handle not inheritable
                        NULL,  // Thread handle not inheritable
                        FALSE, // Set handle inheritance to FALSE
                        0,     // No creation flags
                        NULL,  // Use parent's environment block
                        NULL,  // Use parent's starting directory
                        &si,   // Pointer to STARTUPINFO structure
                        &pi);  // Pointer to PROCESS_INFORMATION structure
      if (result == 0) {
        logger::error("Failed to run the game.");
      }
    }
  } else {
    logger::error("The scene has not yet loaded anything.");
  }

#endif // _WIN32
}
} // namespace sinen
