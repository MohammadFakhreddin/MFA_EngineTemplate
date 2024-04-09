#include "SoundApp.hpp"

#include "RenderBackend.hpp"

using namespace MFA;

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
}

//------------------------------------------------------------------

SoundApp::~SoundApp()
{

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
