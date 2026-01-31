#ifndef IMGUI_IMPL_PARANOIXA_HPP
#define IMGUI_IMPL_PARANOIXA_HPP
#include <imgui.h> // IMGUI_IMPL_API
#ifndef IMGUI_DISABLE
#include <gpu/gpu.hpp>

namespace sinen {

struct ImGuiImplParanoixaInitInfo {
  Allocator *Allocator = nullptr;
  Ptr<gpu::Device> Device = nullptr;
  gpu::TextureFormat ColorTargetFormat = gpu::TextureFormat::Invalid;
  gpu::SampleCount MSAASamples = gpu::SampleCount::x1;
};

IMGUI_IMPL_API bool imGuiImplParanoixaInit(ImGuiImplParanoixaInitInfo *info);
IMGUI_IMPL_API void imGuiImplParanoixaShutdown();
IMGUI_IMPL_API void imGuiImplParanoixaNewFrame();
IMGUI_IMPL_API void
imGuiImplParanoixaPrepareDrawData(ImDrawData *draw_data,
                                  Ptr<gpu::CommandBuffer> command_buffer);
IMGUI_IMPL_API void
imGuiImplParanoixaRenderDrawData(ImDrawData *draw_data,
                                 Ptr<gpu::CommandBuffer> command_buffer,
                                 Ptr<gpu::RenderPass> render_pass,
                                 Ptr<gpu::GraphicsPipeline> pipeline = nullptr);

IMGUI_IMPL_API void imGuiImplParanoixaCreateDeviceObjects();
IMGUI_IMPL_API void imGuiImplParanoixaDestroyDeviceObjects();
IMGUI_IMPL_API void imGuiImplParanoixaCreateFontsTexture();
IMGUI_IMPL_API void imGuiImplParanoixaDestroyFontsTexture();

} // namespace sinen

#endif // #ifndef IMGUI_DISABLE
#endif // IMGUI_IMPL_PARANOIXA_HPP