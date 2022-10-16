#include "editor.hpp"
// imgui
#include <imgui.h>
#include <imgui_impl_sdl.h>
// Added for ImGui
#include <ImGuizmo.h>
#include <TextEditor.h>
#include <imfilebrowser.h>
#include <imgui_markdown.h>

namespace sinen {
std::function<void()> editor::m_editor = nullptr;
std::function<void()> editor::m_markdown = nullptr;
void LinkCallback(ImGui::MarkdownLinkCallbackData data_);
inline ImGui::MarkdownImageData
ImageCallback(ImGui::MarkdownLinkCallbackData data_);

static ImFont *H1 = NULL;
static ImFont *H2 = NULL;
static ImFont *H3 = NULL;

static ImGui::MarkdownConfig mdConfig;

void LinkCallback(ImGui::MarkdownLinkCallbackData data_) {
  std::string url(data_.link, data_.linkLength);
  if (!data_.isImage) {
  }
}

inline ImGui::MarkdownImageData
ImageCallback(ImGui::MarkdownLinkCallbackData data_) {
  // In your application you would load an image based on data_ input. Here we
  // just use the imgui font texture.
  ImTextureID image = ImGui::GetIO().Fonts->TexID;
  // > C++14 can use ImGui::MarkdownImageData imageData{ true, false, image,
  // ImVec2( 40.0f, 20.0f ) };
  ImGui::MarkdownImageData imageData;
  imageData.isValid = true;
  imageData.useLinkCallback = false;
  imageData.user_texture_id = image;
  imageData.size = ImVec2(40.0f, 20.0f);

  // For image resize when available size.x > image width, add
  ImVec2 const contentSize = ImGui::GetContentRegionAvail();
  if (imageData.size.x > contentSize.x) {
    float const ratio = imageData.size.y / imageData.size.x;
    imageData.size.x = contentSize.x;
    imageData.size.y = contentSize.x * ratio;
  }

  return imageData;
}

void ExampleMarkdownFormatCallback(
    const ImGui::MarkdownFormatInfo &markdownFormatInfo_, bool start_) {
  // Call the default first so any settings can be overwritten by our
  // implementation. Alternatively could be called or not called in a switch
  // statement on a case by case basis. See defaultMarkdownFormatCallback
  // definition for furhter examples of how to use it.
  ImGui::defaultMarkdownFormatCallback(markdownFormatInfo_, start_);

  switch (markdownFormatInfo_.type) {
  // example: change the colour of heading level 2
  case ImGui::MarkdownFormatType::HEADING: {
    if (markdownFormatInfo_.level == 2) {
      if (start_) {
        ImGui::PushStyleColor(ImGuiCol_Text,
                              ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
      } else {
        ImGui::PopStyleColor();
      }
    }
    break;
  }
  default: {
    break;
  }
  }
}

void Markdown(const std::string &markdown_) {
  // You can make your own Markdown function with your prefered string container
  // and markdown config. > C++14 can use ImGui::MarkdownConfig mdConfig{
  // LinkCallback, NULL, ImageCallback, ICON_FA_LINK, { { H1, true }, { H2, true
  // }, { H3, false } }, NULL };
  mdConfig.linkCallback = LinkCallback;
  mdConfig.tooltipCallback = NULL;
  mdConfig.imageCallback = ImageCallback;
  mdConfig.linkIcon = "\xef\x83\x81";
  mdConfig.headingFormats[0] = {H1, true};
  mdConfig.headingFormats[1] = {H2, true};
  mdConfig.headingFormats[2] = {H3, false};
  mdConfig.userData = NULL;
  mdConfig.formatCallback = ExampleMarkdownFormatCallback;
  ImGui::Markdown(markdown_.c_str(), markdown_.length(), mdConfig);
}

void MarkdownExample() {
  const std::string markdownText = R"(
# H1 Header: Text and Links
You can add [links like this one to enkisoftware](https://www.enkisoftware.com/) and lines will wrap well.
You can also insert images ![image alt text](image identifier e.g. filename)
Horizontal rules:
***
___
*Emphasis* and **strong emphasis** change the appearance of the text.
## H2 Header: indented text.
  This text has an indent (two leading spaces).
    This one has two.
### H3 Header: Lists
  * Unordered iists
    * Lists can be indented with two extra spaces.
  * Lists can have [links like this one to Avoyd](https://www.avoyd.com/) and *emphasized text*
)";
  Markdown(markdownText);
}

class editor::implements {
public:
  TextEditor texteditor;
  ImGui::FileBrowser file_dialog;
  bool is_run = false;
  bool is_save = false;
};

editor::editor() : m_impl(std::make_unique<editor::implements>()) {}
editor::~editor() {}
void editor::setup() {
  m_impl->is_run = false;
  const static TextEditor::Palette p = {{
      0xff7f7f7f, // Default
      0xffd69c56, // Keyword
      0xff00ff00, // Number
      0xff7070e0, // String
      0xff70a0e0, // Char literal
      0xffffffff, // Punctuation
      0xff408080, // Preprocessor
      0xffaaaaaa, // Identifier
      0xff9bc64d, // Known identifier
      0xffc040a0, // Preproc identifier
      0xff206020, // Comment (single line)
      0xff406020, // Comment (multi line)
      0x00101010, // Background
      0xffe0e0e0, // Cursor
      0x80a06020, // Selection
      0x800020ff, // ErrorMarker
      0x40f08000, // Breakpoint
      0xff707000, // Line number
      0x40000000, // Current line fill
      0x40808080, // Current line fill (inactive)
      0x40a0a0a0, // Current line edge
  }};

  m_impl->texteditor.SetPalette(p);
  m_impl->texteditor.SetLanguageDefinition(
      TextEditor::LanguageDefinition::Lua());
  m_impl->texteditor.SetShowWhitespaces(true);
  m_editor = [&]() {
    auto cpos = m_impl->texteditor.GetCursorPosition();
    ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s | fps:%.2f",
                cpos.mLine + 1, cpos.mColumn + 1,
                m_impl->texteditor.GetTotalLines(),
                m_impl->texteditor.IsOverwrite() ? "Ovr" : "Ins",
                m_impl->texteditor.CanUndo() ? "*" : " ",
                m_impl->texteditor.GetLanguageDefinition().mName.c_str(),
                "aaaa", ImGui::GetIO().Framerate);
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Save", "Ctrl-S", nullptr,
                            m_impl->texteditor.CanUndo())) {
          m_impl->is_save = true;
        }
        if (ImGui::MenuItem("Quit", "Alt-F4"))
          scene::set_state(scene::state::quit);
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Edit")) {
        bool ro = m_impl->texteditor.IsReadOnly();
        if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
          m_impl->texteditor.SetReadOnly(ro);
        ImGui::Separator();

        if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr,
                            !ro && m_impl->texteditor.CanUndo()))
          m_impl->texteditor.Undo();
        if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr,
                            !ro && m_impl->texteditor.CanRedo()))
          m_impl->texteditor.Redo();

        ImGui::Separator();

        if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr,
                            m_impl->texteditor.HasSelection()))
          m_impl->texteditor.Copy();
        if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr,
                            !ro && m_impl->texteditor.HasSelection()))
          m_impl->texteditor.Cut();
        if (ImGui::MenuItem("Delete", "Del", nullptr,
                            !ro && m_impl->texteditor.HasSelection()))
          m_impl->texteditor.Delete();
        if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr,
                            !ro && ImGui::GetClipboardText() != nullptr))
          m_impl->texteditor.Paste();

        ImGui::Separator();

        if (ImGui::MenuItem("Select all", nullptr, nullptr))
          m_impl->texteditor.SetSelection(
              TextEditor::Coordinates(),
              TextEditor::Coordinates(m_impl->texteditor.GetTotalLines(), 0));

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("View")) {
        if (ImGui::MenuItem("Dark palette"))
          m_impl->texteditor.SetPalette(TextEditor::GetDarkPalette());
        if (ImGui::MenuItem("Light palette"))
          m_impl->texteditor.SetPalette(TextEditor::GetLightPalette());
        if (ImGui::MenuItem("Retro blue palette"))
          m_impl->texteditor.SetPalette(TextEditor::GetRetroBluePalette());
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("App")) {
        if (ImGui::MenuItem("Run", "F5", nullptr, true))
          m_impl->is_run = true;
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }
    m_impl->texteditor.Render("Code");
    m_impl->file_dialog.Open();
    m_impl->file_dialog.Display();
  };
  renderer::add_imgui_function(m_editor);
  m_markdown = [&]() { MarkdownExample(); };
}
void imguizmo() {
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
}
void editor::update(float delta_time) {
  if (input::keyboard.get_key_state(key_code::F3) == button_state::Pressed) {
    renderer::toggle_show_imgui();
    if (renderer::is_show_imgui()) {
      renderer::get_imgui_function().clear();
      renderer::add_imgui_function(m_editor);
    }
  }
  if (input::keyboard.get_key_state(key_code::F4) == button_state::Pressed) {
    renderer ::toggle_show_imgui();
    if (renderer::is_show_imgui()) {
      renderer::get_imgui_function().clear();
      renderer::add_imgui_function(m_markdown);
    }
  }
  if (input::keyboard.get_key_state(key_code::F2) == button_state::Pressed) {
    renderer ::toggle_show_imgui();
    if (renderer::is_show_imgui()) {
      renderer::get_imgui_function().clear();
      renderer::add_imgui_function(imguizmo);
    }
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
    auto str = m_impl->texteditor.GetText();
    dstream::write(asset_type::Script,
                   main::get_current_scene_number() + ".lua", str);
    std::cout << str << std::endl;
    m_impl->is_save = false;
  }
}
} // namespace sinen