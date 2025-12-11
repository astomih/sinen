#ifndef IMGUI_IMPL_PARANOIXA_HPP
#define IMGUI_IMPL_PARANOIXA_HPP
#include <imgui.h> // IMGUI_IMPL_API
#ifndef IMGUI_DISABLE
#include <graphics/rhi/rhi.hpp>

namespace sinen {

struct ImGui_ImplParanoixa_InitInfo {
  rhi::Allocator *Allocator = nullptr;
  rhi::Ptr<rhi::Device> Device = nullptr;
  rhi::TextureFormat ColorTargetFormat = rhi::TextureFormat::Invalid;
  rhi::SampleCount MSAASamples = rhi::SampleCount::x1;
};

IMGUI_IMPL_API bool
ImGui_ImplParanoixa_Init(ImGui_ImplParanoixa_InitInfo *info);
IMGUI_IMPL_API void ImGui_ImplParanoixa_Shutdown();
IMGUI_IMPL_API void ImGui_ImplParanoixa_NewFrame();
IMGUI_IMPL_API void Imgui_ImplParanoixa_PrepareDrawData(
    ImDrawData *draw_data, rhi::Ptr<rhi::CommandBuffer> command_buffer);
IMGUI_IMPL_API void ImGui_ImplParanoixa_RenderDrawData(
    ImDrawData *draw_data, rhi::Ptr<rhi::CommandBuffer> command_buffer,
    rhi::Ptr<rhi::RenderPass> render_pass,
    rhi::Ptr<rhi::GraphicsPipeline> pipeline = nullptr);

IMGUI_IMPL_API void ImGui_ImplParanoixa_CreateDeviceObjects();
IMGUI_IMPL_API void ImGui_ImplParanoixa_DestroyDeviceObjects();
IMGUI_IMPL_API void ImGui_ImplParanoixa_CreateFontsTexture();
IMGUI_IMPL_API void ImGui_ImplParanoixa_DestroyFontsTexture();

} // namespace sinen

#endif // #ifndef IMGUI_DISABLE
#endif // IMGUI_IMPL_PARANOIXA_HPP