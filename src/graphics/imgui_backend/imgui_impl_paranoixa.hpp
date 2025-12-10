#ifndef IMGUI_IMPL_PARANOIXA_HPP
#define IMGUI_IMPL_PARANOIXA_HPP
#include <imgui.h> // IMGUI_IMPL_API
#ifndef IMGUI_DISABLE
#include <graphics/paranoixa/paranoixa.hpp>

struct ImGui_ImplParanoixa_InitInfo {
  px::Allocator *Allocator = nullptr;
  px::Ptr<px::Device> Device = nullptr;
  px::TextureFormat ColorTargetFormat = px::TextureFormat::Invalid;
  px::SampleCount MSAASamples = px::SampleCount::x1;
};

IMGUI_IMPL_API bool
ImGui_ImplParanoixa_Init(ImGui_ImplParanoixa_InitInfo *info);
IMGUI_IMPL_API void ImGui_ImplParanoixa_Shutdown();
IMGUI_IMPL_API void ImGui_ImplParanoixa_NewFrame();
IMGUI_IMPL_API void
Imgui_ImplParanoixa_PrepareDrawData(ImDrawData *draw_data,
                                    px::Ptr<px::CommandBuffer> command_buffer);
IMGUI_IMPL_API void ImGui_ImplParanoixa_RenderDrawData(
    ImDrawData *draw_data, px::Ptr<px::CommandBuffer> command_buffer,
    px::Ptr<px::RenderPass> render_pass,
    px::Ptr<px::GraphicsPipeline> pipeline = nullptr);

IMGUI_IMPL_API void ImGui_ImplParanoixa_CreateDeviceObjects();
IMGUI_IMPL_API void ImGui_ImplParanoixa_DestroyDeviceObjects();
IMGUI_IMPL_API void ImGui_ImplParanoixa_CreateFontsTexture();
IMGUI_IMPL_API void ImGui_ImplParanoixa_DestroyFontsTexture();

#endif // #ifndef IMGUI_DISABLE
#endif // IMGUI_IMPL_PARANOIXA_HPP