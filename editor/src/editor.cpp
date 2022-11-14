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

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
namespace sinen {
float mat[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1};
float deltam[16];
vector3 pos, rot, scale;
vector3 position(0.0f, -10.0f, 10.f);
vector3 target = vector3(0, 0, 0);
vector3 up = vector3(0.f, 0.f, 1.f);
void imguizmo() {
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
  ImGuizmo::Manipulate(camera::view().get(), camera::projection().get(),
                       mCurrentGizmoOperation, mCurrentGizmoMode, mat, deltam,
                       nullptr);
  ImGui::SliderFloat3("Position", &position.x, -10.0f, 10.0f);
  ImGui::SliderFloat3("Target", &target.x, -10.0f, 10.0f);
  ImGui::SliderFloat3("Up", &up.x, -1.0f, 1.0f);
  camera::lookat(position, target, up);
  ImGui::End();
}
void inspector() {
  ImGui::SetNextWindowPos({1030, 0});
  ImGui::SetNextWindowSize({250, 720});
  ImGui::Begin("Inspector");
  ImGui::Text("Transform");
  ImGuizmo::DecomposeMatrixToComponents(mat, &pos.x, &rot.x, &scale.x);
  ImGui::DragFloat3("Position", &pos.x);
  ImGui::DragFloat3("Rotation", &rot.x);
  ImGui::DragFloat3("Scale", &scale.x);
  ImGuizmo::RecomposeMatrixFromComponents(&pos.x, &rot.x, &scale.x, mat);
}
void menu() {

  ImGui::SetNextWindowPos({250, 0});
  ImGui::SetNextWindowSize({780, 20});
  ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_MenuBar);
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Save")) {
        std::string str;
        str = "{}";
        rapidjson::Document doc;
        doc.Parse(str.c_str());
        rapidjson::Value actors(rapidjson::kObjectType);
        {
          actors.AddMember("px", pos.x, doc.GetAllocator());
          actors.AddMember("py", pos.y, doc.GetAllocator());
          actors.AddMember("pz", pos.z, doc.GetAllocator());
          actors.AddMember("rx", rot.x, doc.GetAllocator());
          actors.AddMember("ry", rot.y, doc.GetAllocator());
          actors.AddMember("rz", rot.z, doc.GetAllocator());
          actors.AddMember("sx", scale.x, doc.GetAllocator());
          actors.AddMember("sy", scale.y, doc.GetAllocator());
          actors.AddMember("sz", scale.z, doc.GetAllocator());
          actors.AddMember("cpx", position.x, doc.GetAllocator());
          actors.AddMember("cpy", position.y, doc.GetAllocator());
          actors.AddMember("cpz", position.z, doc.GetAllocator());
          actors.AddMember("ctx", target.x, doc.GetAllocator());
          actors.AddMember("cty", target.y, doc.GetAllocator());
          actors.AddMember("ctz", target.z, doc.GetAllocator());
          actors.AddMember("cux", up.x, doc.GetAllocator());
          actors.AddMember("cuy", up.y, doc.GetAllocator());
          actors.AddMember("cuz", up.z, doc.GetAllocator());
        }
        doc.AddMember("Actors", actors, doc.GetAllocator());
        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        doc.Accept(writer);
        file f;
        f.open("data/scene/scene01.json", file::mode::w);
        f.write(buf.GetString(), buf.GetSize(), 1);
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
  renderer::add_imgui_function(imguizmo);
  renderer::add_imgui_function(log_window);
  renderer::add_imgui_function(inspector);
  // renderer::add_imgui_function(func_file_dialog);
  renderer::toggle_show_imgui();
}
void editor::update(float delta_time) {
  texture tex;
  tex.fill_color(palette::white());
  std::shared_ptr<drawable> d3 = std::make_shared<drawable>();

  d3->binding_texture = tex;
  d3->param.proj = camera::projection();
  d3->param.view = camera::view();
  d3->param.world = matrix4(mat);
  d3->vertexIndex = "BOX";
  renderer::draw3d(d3);
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