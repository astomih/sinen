#include <SDL.h>
#include <SDL_rwops.h>
#include <Nen.hpp>

namespace nen
{

    std::string_view AssetReader::Load(const AssetType &assetType, std::string_view name)
    {

        std::string filePath;
        ConvertFilePath(filePath, name, assetType);

        SDL_RWops *file{SDL_RWFromFile(filePath.c_str(), "r")};
#ifndef NEN_NO_EXCEPTION
        if (!file)
            throw std::runtime_error("File open error.");
#endif
        size_t fileLength{static_cast<size_t>(SDL_RWsize(file))};
        void *load{SDL_LoadFile_RW(file, nullptr, 1)};
        std::string_view result(static_cast<char *>(load), fileLength);
        SDL_free(load);
        SDL_FreeRW(file);
        return result;
    }
    void *AssetReader::LoadAsRWops(const AssetType &assetType, std::string_view name)
    {
        std::string filePath;
        ConvertFilePath(filePath, name, assetType);

        SDL_RWops *file{SDL_RWFromFile(filePath.c_str(), "r")};
#ifndef NEN_NO_EXCEPTION
        if (!file)
            throw std::runtime_error("File open error.");
#endif
        return file;
    }
    void AssetReader::ConvertFilePath(std::string &filePath, std::string_view name, const AssetType &assetType)
    {

        switch (assetType)
        {
        case AssetType::Effect:
            filePath += std::string{"Assets/Effect/"} + name.data();
            break;
        case AssetType::Font:
            filePath += std::string{"Assets/Font/"} + name.data();
            break;

        case AssetType::Model:
            filePath += std::string{"Assets/Model/"} + name.data();
            break;
        case AssetType::Music:
            filePath = std::string{"Assets/Music/"} + name.data();
            break;
        case AssetType::Script:
            filePath = std::string{"Assets/Script/"} + name.data();
            break;
        case AssetType::Shader:
            filePath = std::string{"Assets/Shader/"} + name.data();
            break;
        case AssetType::Sound:
            filePath = std::string{"Assets/Sound/"} + name.data();
            break;
        case AssetType::Texture:
            filePath = std::string{"Assets/Texture/"} + name.data();
            break;

        default:
            break;
        }
    }
}