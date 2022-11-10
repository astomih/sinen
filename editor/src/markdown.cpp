#include "markdown.hpp"
#include <SDL.h>
#include <SDL_misc.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_markdown.h>
#include <string>

#if _WIN32
#include <windows.h>
#endif

namespace sinen {
void link_call_back(ImGui::MarkdownLinkCallbackData data_);
inline ImGui::MarkdownImageData
ImageCallback(ImGui::MarkdownLinkCallbackData data_);

static ImFont *H1 = NULL;
static ImFont *H2 = NULL;
static ImFont *H3 = NULL;

static ImGui::MarkdownConfig mdConfig;

void link_call_back(ImGui::MarkdownLinkCallbackData data_) {
  std::string url(data_.link, data_.linkLength);
  if (!data_.isImage) {
#if _WIN32
    ::SDL_OpenURL(url.c_str());
#endif
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
  mdConfig.linkCallback = link_call_back;
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

void markdown() {
  ImGui::Begin("Markdown Viewer");
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
  ImGui::End();
}

} // namespace sinen