#include "SoundApp.hpp"

#include "RenderBackend.hpp"
#include "BedrockPath.hpp"

#include <SDL_mixer.h>

using namespace MFA;

//https://lazyfoo.net/tutorials/SDL/21_sound_effects_and_music/index.php

//------------------------------------------------------------------

SoundApp::SoundApp(std::shared_ptr<SwapChainRenderResource> swapChainResource)
{
    _device = LogicalDevice::Instance;

    _depthResource = std::make_shared<DepthRenderResource>();
    _msaa_Resource = std::make_shared<MSSAA_RenderResource>();
    _displayRenderPass = std::make_shared<DisplayRenderPass>(
    	std::move(swapChainResource),
    	_depthResource,
    	_msaa_Resource
    );

    CreateFontSampler();

    auto pipeline = std::make_shared<TextOverlayPipeline>(_displayRenderPass, _fontSampler);
    _fontRenderer = std::make_unique<ConsolasFontRenderer>(pipeline);
    _textData = _fontRenderer->AllocateTextData();

    _fontRenderer->ResetText(*_textData);
    auto const screenWidth = LogicalDevice::Instance->GetWindowWidth();
    auto const screenHeight = LogicalDevice::Instance->GetWindowHeight();
    _fontRenderer->AddText(
        *_textData,
        "Play 1, 2, 3 or 4 to play a sound effect.",
        screenWidth * 0.5f,
        screenHeight * 0.25f,
        ConsolasFontRenderer::AddTextParams{ .textAlign = ConsolasFontRenderer::TextAlign::Center }
    );

    _fontRenderer->AddText(
        *_textData,
        "Press 9 to play or pause the music.",
        screenWidth * 0.5f,
        screenHeight * 0.5f,
        ConsolasFontRenderer::AddTextParams{ .textAlign = ConsolasFontRenderer::TextAlign::Center }
    );

    _fontRenderer->AddText(
        *_textData,
        "Press 0 to stop the music.",
        screenWidth * 0.5f,
        screenHeight * 0.75f,
        ConsolasFontRenderer::AddTextParams{ .textAlign = ConsolasFontRenderer::TextAlign::Center }
    );

    SetupAudio();

    LogicalDevice::Instance->SDL_EventSignal.Register([&](SDL_Event* event)->void{OnSDL_Event(event);});
}

//------------------------------------------------------------------

SoundApp::~SoundApp() = default;

//------------------------------------------------------------------

void SoundApp::Update(float const deltaTimeSec)
{

}

//------------------------------------------------------------------

void SoundApp::Render(MFA::RT::CommandRecordState & recordState)
{
    _device->BeginCommandBuffer(
    	recordState,
    	RT::CommandBufferType::Compute
    );

    _device->EndCommandBuffer(recordState);

    _device->BeginCommandBuffer(
    	recordState,
    	RT::CommandBufferType::Graphic
    );

    _textData->vertexData->Update(recordState);
    
    _displayRenderPass->Begin(recordState);

    _fontRenderer->Draw(recordState, *_textData);

    _displayRenderPass->End(recordState);

    _device->EndCommandBuffer(recordState);
}

//------------------------------------------------------------------

void SoundApp::CreateFontSampler()
{
    RB::CreateSamplerParams params{};
    params.magFilter = VK_FILTER_LINEAR;
    params.minFilter = VK_FILTER_LINEAR;
    params.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    params.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    params.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    params.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    params.mipLodBias = 0.0f;
    params.compareOp = VK_COMPARE_OP_NEVER;
    params.minLod = 0.0f;
    params.maxLod = 1.0f;
    params.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    
    _fontSampler = RB::CreateSampler(
        _device->GetVkDevice(),
        params
    );
    MFA_ASSERT(_fontSampler != nullptr);
}

//------------------------------------------------------------------

void SoundApp::SetupAudio()
{
    //Initialize SDL_mixer
    _soundSystem = SoundSystem::Instantiate();

    //Load music
    _soundSystem->AddMusic(Path::Instance->Get("sample_audio/beat.wav"), "beatMusic");
    
    //Load sound effects
    _soundSystem->AddChunk(Path::Instance->Get("sample_audio/scratch.wav"), "scratchChunk");

    // _highAudio = Mix_LoadWAV(Path::Instance->Get("sample_audio/high.wav").c_str());
    _soundSystem->AddChunk(Path::Instance->Get("sample_audio/high.wav"), "highChunk");

    // _mediumAudio = Mix_LoadWAV(Path::Instance->Get("sample_audio/medium.wav").c_str());
    _soundSystem->AddChunk(Path::Instance->Get("sample_audio/medium.wav"), "mediumChunk");

    // _lowAudio = Mix_LoadWAV(Path::Instance->Get("sample_audio/low.wav").c_str());
    _soundSystem->AddChunk(Path::Instance->Get("sample_audio/low.wav"), "lowChunk");
}

//------------------------------------------------------------------

void SoundApp::OnSDL_Event(SDL_Event * event)
{
    if(event->type == SDL_KEYDOWN )
    {
        switch(event->key.keysym.sym )
        {
            //Play high sound effect
            case SDLK_1:
            {
                Mix_PlayChannel( -1, _soundSystem->GetChunk("highChunk"), 0 );
                break;
            }
            //Play medium sound effect
            case SDLK_2:
            {
                Mix_PlayChannel( -1, _soundSystem->GetChunk("mediumChunk"), 0 );
                break;
            }
            //Play low sound effect
            case SDLK_3:
            {
                Mix_PlayChannel( -1, _soundSystem->GetChunk("lowChunk"), 0 );
                break;
            }
            //Play scratch sound effect
            case SDLK_4:
            {
                Mix_PlayChannel( -1, _soundSystem->GetChunk("scratchChunk"), 0 );
                break;
            }
            case SDLK_9:
            {
                //If there is no music playing
                if( Mix_PlayingMusic() == 0 )
                {
                    //Play the music
                    Mix_PlayMusic( _soundSystem->GetMusic("beatMusic"), -1 );
                }
                //If music is being played
                else
                {
                    //If the music is paused
                    if( Mix_PausedMusic() == 1 )
                    {
                        //Resume the music
                        Mix_ResumeMusic();
                    }
                    //If the music is playing
                    else
                    {
                        //Pause the music
                        Mix_PauseMusic();
                    }
                }
                break;
            }
            case SDLK_0:
            {
                //Stop the music
                Mix_HaltMusic();
                break;
            }
        }
    }
}

//------------------------------------------------------------------
