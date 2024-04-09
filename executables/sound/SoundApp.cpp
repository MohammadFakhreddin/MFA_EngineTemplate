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

SoundApp::~SoundApp()
{
    // TODO: Wrap the sound
    //Free the sound effects
    Mix_FreeChunk( _scratchAudio );
    Mix_FreeChunk( _highAudio );
    Mix_FreeChunk( _mediumAudio );
    Mix_FreeChunk( _lowAudio );
    _scratchAudio = nullptr;
    _highAudio = nullptr;
    _mediumAudio = nullptr;
    _lowAudio = nullptr;
    
    //Free the music
    Mix_FreeMusic( _music );
    _music = nullptr;

    Mix_Quit();
}

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
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        MFA_LOG_ERROR("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    }

    //Load music
    _music = Mix_LoadMUS(Path::Instance->Get("sample_audio/beat.wav").c_str());
    if( _music == nullptr)
    {
        MFA_LOG_ERROR( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
    }
    
    //Load sound effects
    _scratchAudio = Mix_LoadWAV(Path::Instance->Get("sample_audio/scratch.wav").c_str());
    if( _scratchAudio == nullptr)
    {
        MFA_LOG_ERROR( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
    }
    
    _highAudio = Mix_LoadWAV(Path::Instance->Get("sample_audio/high.wav").c_str());
    if( _highAudio == nullptr)
    {
        MFA_LOG_ERROR( "Failed to load high sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
    }

    _mediumAudio = Mix_LoadWAV(Path::Instance->Get("sample_audio/medium.wav").c_str());
    if( _mediumAudio == nullptr)
    {
        MFA_LOG_ERROR( "Failed to load medium sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
    }

    _lowAudio = Mix_LoadWAV(Path::Instance->Get("sample_audio/low.wav").c_str());
    if( _lowAudio == nullptr)
    {
        MFA_LOG_ERROR( "Failed to load low sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
    }
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
            Mix_PlayChannel( -1, _highAudio, 0 );
            break;
            
            //Play medium sound effect
            case SDLK_2:
            Mix_PlayChannel( -1, _mediumAudio, 0 );
            break;
            
            //Play low sound effect
            case SDLK_3:
            Mix_PlayChannel( -1, _lowAudio, 0 );
            break;
            
            //Play scratch sound effect
            case SDLK_4:
            Mix_PlayChannel( -1, _scratchAudio, 0 );
            break;

            case SDLK_9:
            //If there is no music playing
            if( Mix_PlayingMusic() == 0 )
            {
                //Play the music
                Mix_PlayMusic( _music, -1 );
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
            
            case SDLK_0:
            //Stop the music
            Mix_HaltMusic();
            break;
        }
    }
}

//------------------------------------------------------------------
