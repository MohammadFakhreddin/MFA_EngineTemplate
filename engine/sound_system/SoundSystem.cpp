#include "SoundSystem.hpp"

#include "BedrockAssert.hpp"

#include <filesystem>

namespace MFA
{

    //====================================================================
    
    std::unique_ptr<SoundSystem> SoundSystem::Instantiate()
    {
        return std::make_unique<SoundSystem>();
    }

    //====================================================================

    SoundSystem::SoundSystem()
    {
        //Initialize SDL_mixer
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        {
            MFA_LOG_ERROR("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        }
        MFA_ASSERT(Instance == nullptr);
        Instance = this;
    }

    //====================================================================

    SoundSystem::~SoundSystem()
    {
        MFA_ASSERT(Instance != nullptr);
        Instance = nullptr;

        for (auto & [name, chunk] : _chunkMap)
        {
            MFA_ASSERT(chunk != nullptr);
            Mix_FreeChunk(chunk);
        }

        for (auto & [name, music] : _musicMap)
        {
            MFA_ASSERT(music != nullptr);
            Mix_FreeMusic(music);
        }

        Mix_Quit();
    }

    //====================================================================

    Mix_Chunk * SoundSystem::AddChunk(std::string const & path, std::string const & name)
    {
        MFA_ASSERT(std::filesystem::exists(path));
        auto * chunk = Mix_LoadWAV(path.c_str());   
        MFA_ASSERT(chunk != nullptr);
        auto const findResult = _chunkMap.find(name);
        if (findResult != _chunkMap.end())
        {
            MFA_ASSERT(false);
            Mix_FreeChunk(findResult->second);
            findResult->second = chunk;
        }
        else
        {
            _chunkMap[name] = chunk;
        }
        return chunk;
    }

    //====================================================================

    Mix_Music * SoundSystem::AddMusic(std::string const & path, std::string const & name)
    {
        MFA_ASSERT(std::filesystem::exists(path));
        auto * music = Mix_LoadMUS(path.c_str());   
        MFA_ASSERT(music != nullptr);
        auto const findResult = _musicMap.find(name);
        if (findResult != _musicMap.end())
        {
            MFA_ASSERT(false);
            Mix_FreeMusic(findResult->second);
            findResult->second = music;
        }
        else
        {
            _musicMap[name] = music;
        }
        return music;
    }

    //====================================================================

    Mix_Chunk * SoundSystem::GetChunk(std::string const & name)
    {
        auto const findResult = _chunkMap.find(name);
        if (findResult != _chunkMap.end())
        {
            return findResult->second;
        }
        MFA_ASSERT(false);
        return nullptr;
    }

    //====================================================================

    Mix_Music * SoundSystem::GetMusic(std::string const & name)
    {
        auto const findResult = _musicMap.find(name);
        if (findResult != _musicMap.end())
        {
            return findResult->second;
        }
        MFA_ASSERT(false);
        return nullptr;
    }

    //====================================================================

}