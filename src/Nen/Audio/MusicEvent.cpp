#include <SDL_mixer.h>
#include <Nen.hpp>
namespace nen
{
    MusicEvent::MusicEvent(::Mix_Music *music)
        : music(music)
    {
    }
    MusicEvent::~MusicEvent() = default;

    void MusicEvent::SetVolume(int value)
    {
        Mix_VolumeMusic(value);
    }
}