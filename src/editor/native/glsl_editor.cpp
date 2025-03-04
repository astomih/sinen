#include "glsl_editor.hpp"
#include <sinen/sinen.hpp>
// imgui
#include <imgui.h>
#include <imgui_impl_sdl3.h>
// Added for ImGui
#include <TextEditor.h>

namespace sinen {
std::string glsl_editor::path;
class glsl_editor::impl {
public:
  TextEditor te;
};
const TextEditor::LanguageDefinition &get_glsl() {
  static bool inited = false;
  static TextEditor::LanguageDefinition langDef;
  if (!inited) {
    static const char *const keywords[] = {
        "auto",       "break",     "case",           "char",
        "const",      "continue",  "default",        "do",
        "double",     "else",      "enum",           "extern",
        "float",      "for",       "goto",           "if",
        "inline",     "int",       "long",           "register",
        "restrict",   "return",    "short",          "signed",
        "sizeof",     "static",    "struct",         "switch",
        "typedef",    "union",     "unsigned",       "void",
        "volatile",   "while",     "_Alignas",       "_Alignof",
        "_Atomic",    "_Bool",     "_Complex",       "_Generic",
        "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local",
        "vec3",       "vec4",      "layout",         "mat3",
        "mat4"};
    for (auto &k : keywords)
      langDef.mKeywords.insert(k);

    static const char *const identifiers[] = {
        "abort",   "abs",     "acos",    "asin",     "atan",    "atexit",
        "atof",    "atoi",    "atol",    "ceil",     "clock",   "cosh",
        "ctime",   "div",     "exit",    "fabs",     "floor",   "fmod",
        "getchar", "getenv",  "isalnum", "isalpha",  "isdigit", "isgraph",
        "ispunct", "isspace", "isupper", "kbhit",    "log10",   "log2",
        "log",     "memcmp",  "modf",    "pow",      "putchar", "putenv",
        "puts",    "rand",    "remove",  "rename",   "sinh",    "sqrt",
        "srand",   "strcat",  "strcmp",  "strerror", "time",    "tolower",
        "toupper"};
    for (auto &k : identifiers) {
      TextEditor::Identifier id;
      id.mDeclaration = "Built-in function";
      langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
    }

    langDef.mTokenRegexStrings.push_back(
        std::make_pair<std::string, TextEditor::PaletteIndex>(
            "[ \\t]*#[ \\t]*[a-zA-Z_]+",
            TextEditor::PaletteIndex::Preprocessor));
    langDef.mTokenRegexStrings.push_back(
        std::make_pair<std::string, TextEditor::PaletteIndex>(
            "L?\\\"(\\\\.|[^\\\"])*\\\"", TextEditor::PaletteIndex::String));
    langDef.mTokenRegexStrings.push_back(
        std::make_pair<std::string, TextEditor::PaletteIndex>(
            "\\'\\\\?[^\\']\\'", TextEditor::PaletteIndex::CharLiteral));
    langDef.mTokenRegexStrings.push_back(
        std::make_pair<std::string, TextEditor::PaletteIndex>(
            "[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?",
            TextEditor::PaletteIndex::Number));
    langDef.mTokenRegexStrings.push_back(
        std::make_pair<std::string, TextEditor::PaletteIndex>(
            "[+-]?[0-9]+[Uu]?[lL]?[lL]?", TextEditor::PaletteIndex::Number));
    langDef.mTokenRegexStrings.push_back(
        std::make_pair<std::string, TextEditor::PaletteIndex>(
            "0[0-7]+[Uu]?[lL]?[lL]?", TextEditor::PaletteIndex::Number));
    langDef.mTokenRegexStrings.push_back(
        std::make_pair<std::string, TextEditor::PaletteIndex>(
            "0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?",
            TextEditor::PaletteIndex::Number));
    langDef.mTokenRegexStrings.push_back(
        std::make_pair<std::string, TextEditor::PaletteIndex>(
            "[a-zA-Z_][a-zA-Z0-9_]*", TextEditor::PaletteIndex::Identifier));
    langDef.mTokenRegexStrings.push_back(
        std::make_pair<std::string, TextEditor::PaletteIndex>(
            "[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/"
            "\\;\\,\\.]",
            TextEditor::PaletteIndex::Punctuation));

    langDef.mCommentStart = "/*";
    langDef.mCommentEnd = "*/";
    langDef.mSingleLineComment = "//";

    langDef.mCaseSensitive = true;
    langDef.mAutoIndentation = true;

    langDef.mName = "GLSL";

    inited = true;
  }
  return langDef;
}
std::unique_ptr<glsl_editor::impl> glsl_editor::pimpl =
    std::make_unique<glsl_editor::impl>();
void glsl_editor::display() {
  ImGui::Begin("GLSL Editor", nullptr, ImGuiWindowFlags_MenuBar);
  static char path[256] = {};
  ImGui::InputText("file", path, 256);
  ImGui::SameLine();
  if (ImGui::Button("Open")) {
    auto text = DataStream::open_as_string(AssetType::Shader, path);
    pimpl->te.SetText(text);
  }
  if (ImGui::Button("Compile")) {
    if (!pimpl->te.GetText().empty()) {
      std::string t = pimpl->te.GetText();
      DataStream::write(AssetType::Shader, path, t);
      Shader s;
    }
  }
  pimpl->te.SetLanguageDefinition(get_glsl());
  pimpl->te.SetShowWhitespaces(true);
  auto cpos = pimpl->te.GetCursorPosition();
  ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s ", cpos.mLine + 1,
              cpos.mColumn + 1, pimpl->te.GetTotalLines(),
              pimpl->te.IsOverwrite() ? "Ovr" : "Ins",
              pimpl->te.CanUndo() ? "*" : " ",
              pimpl->te.GetLanguageDefinition().mName.c_str());
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Save")) {
        DataStream::write(AssetType::Script, path, pimpl->te.GetText());
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
      bool ro = pimpl->te.IsReadOnly();
      if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
        pimpl->te.SetReadOnly(ro);
      ImGui::Separator();

      if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr,
                          !ro && pimpl->te.CanUndo()))
        pimpl->te.Undo();
      if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr,
                          !ro && pimpl->te.CanRedo()))
        pimpl->te.Redo();

      ImGui::Separator();

      if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, pimpl->te.HasSelection()))
        pimpl->te.Copy();
      if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr,
                          !ro && pimpl->te.HasSelection()))
        pimpl->te.Cut();
      if (ImGui::MenuItem("Delete", "Del", nullptr,
                          !ro && pimpl->te.HasSelection()))
        pimpl->te.Delete();
      if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr,
                          !ro && ImGui::GetClipboardText() != nullptr))
        pimpl->te.Paste();

      ImGui::Separator();

      if (ImGui::MenuItem("Select all", nullptr, nullptr))
        pimpl->te.SetSelection(
            TextEditor::Coordinates(),
            TextEditor::Coordinates(pimpl->te.GetTotalLines(), 0));

      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
  pimpl->te.Render("Code");
  ImGui::End();
}
std::string glsl_editor::get_text() { return pimpl->te.GetText(); }
void glsl_editor::set_text(const std::string &text) { pimpl->te.SetText(text); }
void glsl_editor::set_script_name(const std::string &path) {
  glsl_editor::path = path;
}
} // namespace sinen
