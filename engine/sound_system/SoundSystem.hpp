#pragma once

#include <SDL_mixer.h>

#include <memory>
#include <unordered_map>
#include <string>

namespace MFA
{
    class SoundSystem
    {
    public:

        static std::unique_ptr<SoundSystem> Instantiate();

        explicit SoundSystem();

        ~SoundSystem();

        Mix_Chunk * AddChunk(std::string const & path, std::string const & name);

        Mix_Music * AddMusic(std::string const & path, std::string const & name);

        Mix_Chunk * GetChunk(std::string const & name);

        Mix_Music * GetMusic(std::string const & name);

        inline static SoundSystem * Instance = nullptr;

    private:

        std::unordered_map<std::string, Mix_Chunk *> _chunkMap{};
        std::unordered_map<std::string, Mix_Music *> _musicMap{};

    };
}