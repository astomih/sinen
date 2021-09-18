#include <SDL_mixer.h>
#include <Nen.hpp>
#include <unordered_map>
#include <string>
namespace nen
{
    class MusicSystem::Impl
    {
    public:
        Impl() = default;
        ~Impl() = default;
        std::unordered_map<std::string, ::Mix_Music *> buffer;
    };
    MusicSystem::MusicSystem()
    {
        impl = std::make_unique<Impl>();
        Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    }

    MusicSystem::~MusicSystem()
    {
        UnloadAllMusic();
        Mix_CloseAudio();
    }

    MusicEvent MusicSystem::PlayMusic(std::string_view name)
    {
        for (auto &i : impl->buffer)
        {
            if (i.first == name)
            {
                Mix_PlayMusic(i.second, -1);
                return MusicEvent(i.second);
            }
        }
        return MusicEvent(nullptr);
    }

    void MusicSystem::LoadMusicFromFile(std::string_view fileName)
    {
        auto rwops = (SDL_RWops *)AssetReader::LoadAsRWops(AssetType::Music, fileName);
        ::Mix_Music *music = ::Mix_LoadMUS_RW(rwops, 1);
        if (music)
        {
            impl->buffer.emplace(std::string(fileName), music);
        }
    }

    void MusicSystem::UnloadAllMusic()
    {
        for (auto i = impl->buffer.begin(); i != impl->buffer.end();)
        {
            Mix_FreeMusic(i->second);
            i = impl->buffer.erase(i);
            if (i != impl->buffer.end())
                i++;
        }
    }

    void MusicSystem::UnloadMusic(std::string_view fileName)
    {
        for (auto i = impl->buffer.begin(); i != impl->buffer.end();)
        {
            if (i->first == fileName)
            {
                Mix_FreeMusic(i->second);
                impl->buffer.erase(i);
                break;
            }
            i++;
        }
    }

}