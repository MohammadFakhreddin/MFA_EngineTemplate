#pragma once

#include <SDL_mixer.h>

#include "RenderTypes.hpp"
#include "LogicalDevice.hpp"
#include "render_resource/SwapChainRenderResource.hpp"
#include "render_pass/DisplayRenderPass.hpp"
#include "render_resource/DepthRenderResource.hpp"
#include "render_resource/MSAA_RenderResource.hpp"
#include "BufferTracker.hpp"
#include "utils/ConsolasFontRenderer.hpp"

class SoundApp
{
public:

    explicit SoundApp(std::shared_ptr<MFA::SwapChainRenderResource> swapChainResource);

    ~SoundApp();

    void Update(float deltaTimeSec);

    void Render(MFA::RT::CommandRecordState & recordState);

private:

    void CreateFontSampler();

    void SetupAudio();

    void OnSDL_Event(SDL_Event * event);

    MFA::LogicalDevice * _device = nullptr;

    std::shared_ptr<MFA::DepthRenderResource> _depthResource{};
    
    std::shared_ptr<MFA::MSSAA_RenderResource> _msaa_Resource{};

    std::shared_ptr<MFA::DisplayRenderPass> _displayRenderPass{};

    std::unique_ptr<MFA::ConsolasFontRenderer> _fontRenderer{};

    std::shared_ptr<MFA::RT::SamplerGroup> _fontSampler{};

    std::unique_ptr<MFA::ConsolasFontRenderer::TextData> _textData{};

    //The music that will be played
    Mix_Music * _music = nullptr;

    //The sound effects that will be used
    Mix_Chunk * _scratchAudio = nullptr;
    Mix_Chunk * _highAudio = nullptr;
    Mix_Chunk * _mediumAudio = nullptr;
    Mix_Chunk * _lowAudio = nullptr;

};