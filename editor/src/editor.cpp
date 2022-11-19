#include "editor.hpp"
#include <SDL.h>
#ifdef main
#undef main
#endif
#include <imgui.h>
#include <imgui_impl_sdl.h>
// Added for ImGui
#include <ImGuizmo.h>

#include "file_dialog.hpp"
#include "log_window.hpp"
#include "markdown.hpp"
#include "texteditor.hpp"

#include <sinen/sinen.hpp>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
namespace sinen {
float deltam[16];
camera::sub editor::main_camera_clone;
std::vector<actor> editor::m_actors;
std::vector<float *> editor::m_matrices;
int editor::index = 0;
void editor::gizmo() {
  ImGui::SetNextWindowPos({0, 0});
  ImGui::SetNextWindowSize({250, 360});
  ImGui::Begin("Camera", nullptr,
               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
  ImGuizmo::BeginFrame();
  static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
  static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
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
    ImGuizmo::Manipulate(camera::view().get(), camera::projection().get(),
                         mCurrentGizmoOperation, mCurrentGizmoMode,
                         m_matrices[index], deltam, nullptr);
  }
  ImGui::SliderFloat3("Position", &main_camera_clone.position().x, -10.0f,
                      10.0f);
  ImGui::SliderFloat3("Target", &main_camera_clone.target().x, -10.0f, 10.0f);
  ImGui::SliderFloat3("Up", &main_camera_clone.up().x, -1.0f, 1.0f);
  camera::lookat(main_camera_clone.position(), main_camera_clone.target(),
                 main_camera_clone.up());
  ImGui::End();
}
void editor::inspector() {
  ImGui::SetNextWindowPos({1030, 0});
  ImGui::SetNextWindowSize({250, 720});
  ImGui::Begin("Inspector");
  ImGui::Text("Transform");
  vector3 pos, rot, scale;
  if (m_actors.size() > 0) {
    ImGuizmo::DecomposeMatrixToComponents(m_matrices[index], &pos.x, &rot.x,
                                          &scale.x);
    ImGui::DragFloat3("Position", &pos.x);
    ImGui::DragFloat3("Rotation", &rot.x);
    ImGui::DragFloat3("Scale", &scale.x);
    ImGuizmo::RecomposeMatrixFromComponents(&pos.x, &rot.x, &scale.x,
                                            m_matrices[index]);
    m_actors[index].set_position(pos);
    m_actors[index].set_rotation(rot);
    m_actors[index].set_scale(scale);
  }

  ImGui::Separator();
  ImGui::Text("Actors");
  for (int i = 0; i < m_actors.size(); i++)
    ImGui::RadioButton(std::string("actor" + std::to_string(i)).c_str(), &index,
                       i);
  static int item = 1;
  /* ImGui::Combo("combo", &item, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0"); */
  if (ImGui::Button("Add Actor")) {
    m_actors.push_back(actor());
    auto m = matrix4::identity;
    float f[16];
    memcpy(f, m.mat, sizeof(float) * 16);
    m_matrices.push_back(f);
  }

  ImGui::End();
}
void editor::menu() {

  ImGui::SetNextWindowPos({250, 0});
  ImGui::SetNextWindowSize({780, 20});
  ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_MenuBar);
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Save")) {
        std::string str;
        str = "{}";
        json j;
        j.parse(str);
        auto camera_data = j.create_object();
        {
          camera_data.add_member("cpx", main_camera_clone.position().x);
          camera_data.add_member("cpy", main_camera_clone.position().y);
          camera_data.add_member("cpz", main_camera_clone.position().z);
          camera_data.add_member("ctx", main_camera_clone.target().x);
          camera_data.add_member("cty", main_camera_clone.target().y);
          camera_data.add_member("ctz", main_camera_clone.target().z);
          camera_data.add_member("cux", main_camera_clone.up().x);
          camera_data.add_member("cuy", main_camera_clone.up().y);
          camera_data.add_member("cuz", main_camera_clone.up().z);
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
      }
      ImGui::EndMenu();
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
  renderer::add_imgui_function(texteditor);
  renderer::add_imgui_function(markdown);
  renderer::add_imgui_function(gizmo);
  renderer::add_imgui_function(log_window);
  renderer::add_imgui_function(inspector);
  // renderer::add_imgui_function(func_file_dialog);
  renderer::toggle_show_imgui();
}
void editor::update(float delta_time) {
  for (int i = 0; i < m_actors.size(); i++) {
    texture tex;
    tex.fill_color(palette::white());
    std::shared_ptr<drawable> d3 = std::make_shared<drawable>();

    d3->binding_texture = tex;
    d3->param.proj = camera::projection();
    d3->param.view = camera::view();
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
    m_impl->is_save = true;
  }
  if (m_impl->is_save) {
    auto str = get_text();
    data_stream::write(asset_type::Script,
                       main::get_current_scene_number() + ".lua", str);
    std::cout << str << std::endl;
    m_impl->is_save = false;
  }
  if (input::keyboard.get_key_state(key_code::F3) == button_state::Pressed) {
    renderer::toggle_show_imgui();
  }
}
} // namespace sinen