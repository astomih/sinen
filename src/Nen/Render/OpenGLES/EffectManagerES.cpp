#include <nen.hpp>
#if defined(EMSCRIPTEN) || defined(MOBILE)
#include "EffectManagerES.h"
#include "ESRenderer.h"
#include <Effekseer.h>
#include <EffekseerRendererGL.h>
#include "../../Effect/CustomTextureLoader.h"
#include <EffekseerRenderer/EffekseerRendererGL.MaterialLoader.h>
#include <EffekseerRenderer/EffekseerRendererGL.RendererImplemented.h>
#include <EffekseerRenderer/GraphicsDevice.h>
#include <fstream>
#include <sstream>

namespace nen::es
{
    class CustomFileReader : public Effekseer::FileReader
    {
        uint8_t *fileData;
        size_t fileSize;
        int currentPosition;

    public:
        CustomFileReader(uint8_t *fileData, size_t fileSize) : fileData(fileData), fileSize(fileSize), currentPosition(0) {}
        ~CustomFileReader() { free(fileData); }
        size_t Read(void *buffer, size_t size)
        {
            if (currentPosition + size > fileSize)
            {
                size = fileSize - currentPosition;
            }
            memcpy(buffer, fileData + currentPosition, size);
            currentPosition += size;
            return size;
        }
        void Seek(int position) { currentPosition = position; }
        int GetPosition() { return currentPosition; }
        size_t GetLength() { return fileSize; }
    };

    class CustomFileInterface : public Effekseer::FileInterface
    {
    public:
        Effekseer::FileReader *OpenRead(const EFK_CHAR *path, bool isRequired)
        {
            std::cout << "INFO: OpenRead() called." << std::endl;
            // Request to load file
            std::array<char, 260> path8;
            Effekseer::ConvertUtf16ToUtf8(path8.data(), static_cast<int32_t>(path8.size()), path);
            std::ifstream ifs(path8.data());
            if (ifs.fail())
            {
                std::cout << "ERROR: Failed file open." << std::endl;
            }
            std::string fdata = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
            ifs.close();
            uint8_t *fileData = reinterpret_cast<uint8_t *>(fdata.data());
            int fileSize = fdata.size();

            if (fileData == nullptr)
            {
                return nullptr;
            }

            return new CustomFileReader(fileData, fileSize);
        }

        Effekseer::FileReader *OpenRead(const EFK_CHAR *path) override { return OpenRead(path, true); }

        Effekseer::FileReader *TryOpenRead(const EFK_CHAR *path) override { return OpenRead(path, false); }

        Effekseer::FileWriter *OpenWrite(const EFK_CHAR *path) override { return nullptr; }
    };
    static CustomFileInterface fileInterface;

    EffectManagerES::EffectManagerES(ESRenderer *renderer)
        : mRenderer(renderer)
    {
    }
    void EffectManagerES::Init()
    {
        // Create a renderer of effects
        renderer = ::EffekseerRendererGL::Renderer::Create(
            8000, EffekseerRendererGL::OpenGLDeviceType::OpenGLES3);

        // Sprcify rendering modules
        manager->SetSpriteRenderer(renderer->CreateSpriteRenderer());
        manager->SetRibbonRenderer(renderer->CreateRibbonRenderer());
        manager->SetRingRenderer(renderer->CreateRingRenderer());
        manager->SetTrackRenderer(renderer->CreateTrackRenderer());
        manager->SetModelRenderer(renderer->CreateModelRenderer());

        // Specify a texture, model and material loader
        manager->SetTextureLoader(Effekseer::MakeRefPtr<CustomTextureLoader>(renderer->GetGraphicsDevice().Get()));
        manager->SetModelLoader(renderer->CreateModelLoader(&fileInterface));
        //manager->SetModelLoader(renderer->CreateModelLoader());
        //manager->SetMaterialLoader(renderer->CreateMaterialLoader());
        //manager->SetCurveLoader(Effekseer::MakeRefPtr<Effekseer::CurveLoader>(&fileInterface));
        //
        manager->SetMaterialLoader(Effekseer::MakeRefPtr<EffekseerRendererGL::MaterialLoader>(
            renderer->GetGraphicsDevice().DownCast<EffekseerRendererGL::Backend::GraphicsDevice>(), &fileInterface, false));

		// Specify a position of view
		auto g_position = ::Effekseer::Vector3D(0.0f, 30.f, 5.f);

		// Specify a projection matrix
		renderer->SetProjectionMatrix(
			::Effekseer::Matrix44().PerspectiveFovRH(Math::ToRadians(90.f), Window::Size.x / Window::Size.y, 0.01f, 10000.f));

		auto m2 = ::Effekseer::Matrix44().LookAtRH(g_position, ::Effekseer::Vector3D(0.0f, -5.f, -10.f), ::Effekseer::Vector3D(0.0f, 1.0f, 0.0f));
		// Specify a camera matrix
		renderer->SetCameraMatrix(m2);
    }
}
#endif