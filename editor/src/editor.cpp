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
#include <imfilebrowser.h>

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
void editor::load_scene(const std::string &path) {
  index = 0;
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
    m_actors.resize(j["Actors"]["size"].get_int32());
    m_matrices.resize(j["Actors"]["size"].get_int32());
    texture tex;
    tex.fill_color(palette::white());
    for (int i = 0; i < m_actors.size(); i++) {
      auto act = j["Actors"][std::string("Actor") + std::to_string(i)];
      m_actors[i].set_position(vector3(act["Position"]["x"].get_float(),
                                       act["Position"]["y"].get_float(),
                                       act["Position"]["z"].get_float()));
      m_actors[i].set_rotation(vector3(act["Rotation"]["x"].get_float(),
                                       act["Rotation"]["y"].get_float(),
                                       act["Rotation"]["z"].get_float()));
      m_actors[i].set_scale(vector3(act["Scale"]["x"].get_float(),
                                    act["Scale"]["y"].get_float(),
                                    act["Scale"]["z"].get_float()));
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
  auto actors = j.create_object();
  {
    {
      auto act = j.create_object();
      act.set_int32(m_actors.size());
      actors.add_member("size", act);
    }
    for (int i = 0; i < m_actors.size(); i++) {
      auto act = j.create_object();
      {
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
      }
      actors.add_member(std::string("Actor" + std::to_string(i)), act);
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
void editor::menu() {
  ImGui::SetNextWindowPos({250, 0});
  ImGui::SetNextWindowSize({780, 20});
  ImGui::Begin("Menu", nullptr,
               ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoResize);
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
      if (ImGui::MenuItem("Exit", "Alt+F4")) {
#if !defined(EMSCRIPTEN)
        std::exit(0);
#else
        emscripten_force_exit(0);
#endif
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
  m_impl->is_run = false;
  renderer::add_imgui_function(menu);
  renderer::add_imgui_function(markdown);
  renderer::add_imgui_function(log_window);
  renderer::add_imgui_function(inspector);
  renderer::add_imgui_function(texteditor);
  // renderer::add_imgui_function(func_file_dialog);
  renderer::toggle_show_imgui();
}
void editor::update(float delta_time) {
  if (request_pop_func) {
    renderer::get_imgui_function().pop_back();
    request_pop_func = false;
  }
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
    save_scene(this->current_file_name);
  }
  if (input::keyboard.get_key_state(key_code::F3) == button_state::Pressed) {
    renderer::toggle_show_imgui();
  }
  if (input::keyboard.get_key_state(key_code::F5) == button_state::Pressed) {
#ifdef _WIN32
    if (!this->current_file_name.empty()) {
      STARTUPINFO si;
      PROCESS_INFORMATION pi;
      ZeroMemory(&si, sizeof(si));
      si.cb = sizeof(si);
      ZeroMemory(&pi, sizeof(pi));
      std::string commandlp = std::string(std::string("game.exe ") +
                                          std::string(this->current_file_name));
      // Start the child process.
      CreateProcess(NULL, // No module name (use command line)
                    (LPSTR)commandlp.c_str(), // Command line
                    NULL,                     // Process handle not inheritable
                    NULL,                     // Thread handle not inheritable
                    FALSE,                    // Set handle inheritance to FALSE
                    0,                        // No creation flags
                    NULL,                     // Use parent's environment block
                    NULL,                     // Use parent's starting directory
                    &si,  // Pointer to STARTUPINFO structure
                    &pi); // Pointer to PROCESS_INFORMATION structure
    }

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
