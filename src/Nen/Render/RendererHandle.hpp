#include <Render/Renderer.hpp>

namespace nen
{
    class RendererHandle
    {
    public:
        static void SetRenderer(std::shared_ptr<Renderer> renderer)
        {
            mRenderer = renderer;
        }

        static std::shared_ptr<Renderer> GetRenderer() { return mRenderer; }

    private:
        RendererHandle() = default;
        static std::shared_ptr<Renderer> mRenderer;
    };
}