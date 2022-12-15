#include "editor.hpp"
#include <SDL.h>
#ifdef main
#undef main
#endif
#include <imgui.h>
#include <imgui_impl_sdl.h>
// Added for ImGui
#include <ImGuizmo.h>

#include "log_window.hpp"
#include "markdown.hpp"
#include "texteditor.hpp"

#include <sinen/sinen.hpp>

#include <component/draw3d_component.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#if defined(_WIN32)
#include <windows.h>
#endif
namespace sinen {
std::vector<actor> editor::m_actors;
std::vector<matrix4> editor::m_matrices;
int editor::index = 0;
void editor::inspector() {
  ImGui::SetNextWindowPos({1030, 0});
  ImGui::SetNextWindowSize({250, 720});
  ImGui::Begin("Inspector", nullptr,
               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
  ImGuizmo::BeginFrame();
  static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
  static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
  if (ImGui::IsKeyPressed(90))
    mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
  if (ImGui::IsKeyPressed(69))
    mCurrentGizmoOperation = ImGuizmo::ROTATE;
  if (ImGui::IsKeyPressed(82)) // r Key
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
  ImGuizmo::SetRect(0, 0, window::size().x, window::size().y);
  ImGuizmo::AllowAxisFlip(false);
  if (m_matrices.size() > 0) {
    ImGuizmo::Manipulate(scene::main_camera().view().get(),
                         scene::main_camera().projection().get(),
                         mCurrentGizmoOperation, mCurrentGizmoMode,
                         m_matrices[index].mat.m16);
  }
  ImGui::Text("Transform");
  vector3 pos, rot, scale;
  if (m_actors.size() > 0) {
    ImGuizmo::DecomposeMatrixToComponents(m_matrices[index].mat.m16, &pos.x,
                                          &rot.x, &scale.x);
    ImGui::DragFloat3("Position", &pos.x);
    ImGui::DragFloat3("Rotation", &rot.x);
    ImGui::DragFloat3("Scale", &scale.x);
    ImGuizmo::RecomposeMatrixFromComponents(&pos.x, &rot.x, &scale.x,
                                            m_matrices[index].mat.m16);
    m_actors[index].set_position(pos);
    m_actors[index].set_rotation(rot);
    m_actors[index].set_scale(scale);
  }
  ImGui::Separator();
  ImGui::Text("Components");
  if (m_actors.size() > 0) {
    static bool d3d = false;
    ImGui::Checkbox("draw3d_component", &d3d);
    if (ImGui::Button("Add component")) {
    }
  }
  ImGui::Text("Actors");
  for (int i = 0; i < m_actors.size(); i++)
    ImGui::RadioButton(std::string("actor" + std::to_string(i)).c_str(), &index,
                       i);
  static int item = 1;
  if (ImGui::Button("Add Actor")) {
    m_actors.push_back(std::move(actor{}));
    auto m = matrix4::identity;
    m_matrices.push_back(m);
  }

  ImGui::End();
}
void editor::load_scene() {
  index = 0;
  auto str = data_stream::open_as_string(asset_type::Scene, "scene01.json");
  json j;
  j.parse(str);
  // Camera
  {
    auto camera_data = j["Camera"];
    scene::main_camera().position().x = camera_data["cpx"].get_float();
    scene::main_camera().position().y = camera_data["cpy"].get_float();
    scene::main_camera().position().z = camera_data["cpz"].get_float();
    scene::main_camera().target().x = camera_data["ctx"].get_float();
    scene::main_camera().target().y = camera_data["cty"].get_float();
    scene::main_camera().target().z = camera_data["ctz"].get_float();
    scene::main_camera().up().x = camera_data["cux"].get_float();
    scene::main_camera().up().y = camera_data["cuy"].get_float();
    scene::main_camera().up().z = camera_data["cuz"].get_float();
    scene::main_camera().lookat(scene::main_camera().position(),
                                scene::main_camera().target(),
                                scene::main_camera().up());
  }
  // Actors
  {
    m_actors.clear();
    m_matrices.clear();
    m_actors.resize(j["Actors"].size());
    m_matrices.resize(j["Actors"].size());
    texture tex;
    tex.fill_color(palette::white());
    for (int i = 0; i < m_actors.size(); i++) {
      auto act = j["Actors"][std::string("Actor") + std::to_string(i)];
      m_actors[i].set_position(vector3(
          act["px"].get_float(), act["py"].get_float(), act["pz"].get_float()));
      m_actors[i].set_rotation(vector3(
          act["rx"].get_float(), act["ry"].get_float(), act["rz"].get_float()));
      m_actors[i].set_scale(vector3(
          act["sx"].get_float(), act["sy"].get_float(), act["sz"].get_float()));
      auto &d3 = m_actors[i].create_component<draw3d_component>();
      d3.set_texture(tex);
      d3.set_vertex_name("BOX");
      vector3 pos, rot, scale;
      pos = m_actors[i].get_position();
      rot = m_actors[i].get_rotation();
      scale = m_actors[i].get_scale();
      ImGuizmo::RecomposeMatrixFromComponents(&pos.x, &rot.x, &scale.x,
                                              m_matrices[i].get());
    }
  }
  logger::info("Scene loaded");
}
void editor::save_scene() {
  std::string str;
  str = "{}";
  json j;
  j.parse(str);
  auto camera_data = j.create_object();
  {
    camera_data.add_member("cpx", scene::main_camera().position().x);
    camera_data.add_member("cpy", scene::main_camera().position().y);
    camera_data.add_member("cpz", scene::main_camera().position().z);
    camera_data.add_member("ctx", scene::main_camera().target().x);
    camera_data.add_member("cty", scene::main_camera().target().y);
    camera_data.add_member("ctz", scene::main_camera().target().z);
    camera_data.add_member("cux", scene::main_camera().up().x);
    camera_data.add_member("cuy", scene::main_camera().up().y);
    camera_data.add_member("cuz", scene::main_camera().up().z);
  }
  j.add_member("Camera", camera_data);
  auto actors = j.create_object();
  {
    for (int i = 0; i < m_actors.size(); i++) {
      auto act = j.create_object();
      {
        vector3 pos = m_actors[i].get_position();
        vector3 rot = m_actors[i].get_rotation();
        vector3 scale = m_actors[i].get_scale();
        act.add_member("px", pos.x);
        act.add_member("py", pos.y);
        act.add_member("pz", pos.z);
        act.add_member("rx", rot.x);
        act.add_member("ry", rot.y);
        act.add_member("rz", rot.z);
        act.add_member("sx", scale.x);
        act.add_member("sy", scale.y);
        act.add_member("sz", scale.z);
      }
      actors.add_member(std::string("Actor" + std::to_string(i)), act);
    }
  }
  j.add_member("Actors", actors);
  auto s = j.to_string();
  file f;
  f.open("data/scene/scene01.json", file::mode::w);
  f.write(s.c_str(), s.size(), 1);
  f.close();
  logger::info("Scene saved");
}
void editor::menu() {
  ImGui::SetNextWindowPos({250, 0});
  ImGui::SetNextWindowSize({780, 20});
  ImGui::Begin("Menu", nullptr,
               ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoResize);
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Load")) {
        load_scene();
        ImGui::EndMenu();
      }
      if (ImGui::MenuItem("Save", "Ctrl+S")) {
        save_scene();
        ImGui::EndMenu();
      }
      if (ImGui::MenuItem("Exit", "Alt+F4")) {
#if !defined(EMSCRIPTEN)
        std::exit(0);
#else
        emscripten_force_exit(0);
#endif
      }
    }
    ImGui::EndMenuBar();
  }
  ImGui::End();
}

class editor::implements {
public:
  bool is_run = false;
  bool is_save = false;
};

editor::editor() : m_impl(std::make_unique<editor::implements>()) {}
editor::~editor() {}
void editor::setup() {
  m_impl->is_run = false;
  renderer::add_imgui_function(menu);
  renderer::add_imgui_function(markdown);
  renderer::add_imgui_function(log_window);
  renderer::add_imgui_function(inspector);
  // renderer::add_imgui_function(texteditor);
  // renderer::add_imgui_function(func_file_dialog);
  renderer::toggle_show_imgui();
  load_scene();
}
void editor::update(float delta_time) {
  for (int i = 0; i < m_actors.size(); i++) {
    texture tex;
    tex.fill_color(palette::white());
    std::shared_ptr<drawable> d3 = std::make_shared<drawable>();

    d3->binding_texture = tex;
    d3->param.proj = scene::main_camera().projection();
    d3->param.view = scene::main_camera().view();
    d3->param.world = matrix4(m_matrices[i]);
    d3->vertexIndex = "BOX";
    renderer::draw3d(d3);
  }
  if (renderer::is_show_imgui() &&
      input::keyboard.get_key_state(key_code::F5) == button_state::Pressed) {
    m_impl->is_run = true;
  }
  if (m_impl->is_run) {
    m_impl->is_run = false;
  }
  if (renderer::is_show_imgui() &&
      input::keyboard.is_key_down(key_code::LCTRL) &&
      input::keyboard.get_key_state(key_code::S) == button_state::Pressed) {
    // m_impl->is_save = true;
  }
  if (m_impl->is_save) {
    auto str = get_text();
    data_stream::write(asset_type::Script,
                       main::get_current_scene_number() + ".lua", str);
    std::cout << str << std::endl;
    m_impl->is_save = false;
  }
  if (renderer::is_show_imgui() &&
      input::keyboard.is_key_down(key_code::LCTRL) &&
      input::keyboard.get_key_state(key_code::S) == button_state::Pressed) {
    save_scene();
  }
  if (input::keyboard.get_key_state(key_code::F3) == button_state::Pressed) {
    renderer::toggle_show_imgui();
  }
  if (input::keyboard.get_key_state(key_code::F5) == button_state::Pressed) {
#ifdef _WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    const char *commandlp = "game.exe";
    // Start the child process.
    CreateProcess(NULL,             // No module name (use command line)
                  (LPSTR)commandlp, // Command line
                  NULL,             // Process handle not inheritable
                  NULL,             // Thread handle not inheritable
                  FALSE,            // Set handle inheritance to FALSE
                  0,                // No creation flags
                  NULL,             // Use parent's environment block
                  NULL,             // Use parent's starting directory
                  &si,              // Pointer to STARTUPINFO structure
                  &pi);             // Pointer to PROCESS_INFORMATION structure

#endif // _WIN32
  }

  // Camera moved by mouse
  {
    // Normalize camera angle vector
    auto vec = scene::main_camera().target() - scene::main_camera().position();
    if (vec.length() > 0.1) {
      vec.normalize();
      scene::main_camera().position() +=
          vec * input::mouse.get_scroll_wheel().y;
    }
    static vector2 prev = vector2();
    if (input::mouse.get_button_state(mouse_code::RIGHT) ==
        button_state::Held) {
      auto pos = prev - input::mouse.get_position();
      if (input::keyboard.is_key_down(key_code::LSHIFT)) {
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
    prev = input::mouse.get_position();
  }
  scene::main_camera().lookat(scene::main_camera().position(),
                              scene::main_camera().target(),
                              scene::main_camera().up());
}
} // namespace sinen
