#pragma once

namespace
{
    struct Mix_Music;
}
namespace nen
{
    class MusicEvent
    {
    public:
        MusicEvent(::Mix_Music* music);
        ~MusicEvent();

        void SetVolume(int volume);
        

    private:
        ::Mix_Music *music;
    };

} // namespace nen
