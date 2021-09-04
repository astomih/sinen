#pragma once
#include "MusicEvent.hpp"
#include <string>
#include <memory>

namespace nen
{
    class MusicSystem
    {
    public:
        MusicSystem();
        ~MusicSystem();

        MusicEvent PlayMusic(std::string_view name);

        void LoadMusicFromFile(std::string_view fileName);
        void UnloadAllMusic();
        void UnloadMusic(std::string_view name);

        std::string_view name;

    private:

        class Impl;
        std::unique_ptr<Impl> impl;
    };
} // namespace nen
