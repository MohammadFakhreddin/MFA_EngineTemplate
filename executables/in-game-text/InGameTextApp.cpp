#include "InGameTextApp.hpp"

#include "RenderBackend.hpp"

using namespace MFA;

//------------------------------------------------------------------

InGameTextApp::InGameTextApp(std::shared_ptr<SwapChainRenderResource> swapChainResource)
{
    _device = LogicalDevice::Instance;

    _depthResource = std::make_shared<DepthRenderResource>();
    _msaa_Resource = std::make_shared<MSSAA_RenderResource>();
    _displayRenderPass = std::make_shared<DisplayRenderPass>(
    	std::move(swapChainResource),
    	_depthResource,
    	_msaa_Resource
    );

    CreateTextVertexBuffer();
    CreateFontTextureBuffer();
    CreateFontSampler();
    _pipeline = std::make_unique<TextOverlayPipeline>(_displayRenderPass, _fontSampler);
    _descriptorSet = _pipeline->CreateDescriptorSet(*_fontTexture);
}

//------------------------------------------------------------------

InGameTextApp::~InGameTextApp()
{

}

//------------------------------------------------------------------

void InGameTextApp::Update(float deltaTimeSec)
{
    _letterCount = 0;
    auto const screenWidth = LogicalDevice::Instance->GetWindowWidth();
    auto const screenHeight = LogicalDevice::Instance->GetWindowHeight();
    AddText("Hello", screenWidth * 0.5f, screenHeight * 0.5f, AddTextParams {.textAlign = TextAlign::Center});
    AddText("Text overlay test", 0, 0, AddTextParams {.textAlign = TextAlign::Left});
}

//------------------------------------------------------------------

void InGameTextApp::Render(MFA::RT::CommandRecordState & recordState)
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

    _vertexTracker->Update(recordState);

    _displayRenderPass->Begin(recordState);

    _pipeline->BindPipeline(recordState);
    
    RB::AutoBindDescriptorSet(
        recordState,
        RB::UpdateFrequency::PerPipeline,
        _descriptorSet.descriptorSets[0]
    );
    
    RB::BindVertexBuffer(recordState, *_vertexTracker->LocalBuffer().buffers[recordState.frameIndex]);

    // One draw command for every character. This is okay for a debug overlay, but not optimal
    // In a real-world application one would try to batch draw commands
    // for (uint32_t i = 0; i < _letterCount; i++) 
    // {
    //     vkCmdDraw(recordState.commandBuffer, 4, 1, i * 4, 0);
    // }
    vkCmdDraw(recordState.commandBuffer, _letterCount * 4, 1, 0, 0);

    _displayRenderPass->End(recordState);

    _device->EndCommandBuffer(recordState);
}

//------------------------------------------------------------------

void InGameTextApp::CreateTextVertexBuffer()
{
    auto const vertexBuffer = RB::CreateVertexBufferGroup(
        _device->GetVkDevice(), 
        _device->GetPhysicalDevice(), 
        TEXTOVERLAY_MAX_CHAR_COUNT * sizeof(glm::vec4),
        LogicalDevice::Instance->GetMaxFramePerFlight()
    );

    auto const vertexStageBuffer = RB::CreateStageBuffer(
        _device->GetVkDevice(),
        _device->GetPhysicalDevice(),
        vertexBuffer->bufferSize,
        vertexBuffer->buffers.size()
    );

    _vertexTracker = std::make_unique<LocalBufferTracker<VertexData>>(
        vertexBuffer,
        vertexStageBuffer,
        VertexData{}
    );
}

//------------------------------------------------------------------

void InGameTextApp::CreateFontTextureBuffer()
{
    auto const fontWidth = STB_FONT_consolas_24_latin1_BITMAP_WIDTH;
    auto const fontHeight = STB_FONT_consolas_24_latin1_BITMAP_HEIGHT;

    uint8_t font24pixels[fontHeight][fontWidth];
    stb_font_consolas_24_latin1(_stbFontData, font24pixels, fontHeight);

    AS::Texture cpuTexture {
        Asset::Texture::Format::UNCOMPRESSED_UNORM_R8_LINEAR,
        1,
        1,
        sizeof(font24pixels)
    };
    
    cpuTexture.addMipmap(
        Asset::Texture::Dimensions {
            .width = fontWidth,
            .height = fontHeight,
            .depth = 1
        },
        &font24pixels[0][0], 
        sizeof(font24pixels)
    );

    auto commandBuffer = RB::BeginSingleTimeCommand(
        _device->GetVkDevice(), 
        _device->GetGraphicCommandPool()
    );
    
    auto const [fontTexture, stageBuffer] = RB::CreateTexture(
        cpuTexture,         
        _device->GetVkDevice(),
        _device->GetPhysicalDevice(),
        commandBuffer
    );
    MFA_ASSERT(fontTexture != nullptr);
    _fontTexture = std::move(fontTexture);
    
    RB::EndAndSubmitSingleTimeCommand(
        _device->GetVkDevice(), 
        _device->GetGraphicCommandPool(),
        _device->GetGraphicQueue(),
        commandBuffer
    );
}

//------------------------------------------------------------------

void InGameTextApp::CreateFontSampler()
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

void InGameTextApp::AddText(
    std::string_view const & text, 
    float x, 
    float y, 
    AddTextParams params
)
{
    auto const windowWidth = static_cast<float>(LogicalDevice::Instance->GetWindowWidth());
    auto const windowHeight = static_cast<float>(LogicalDevice::Instance->GetWindowHeight());

    const uint32_t firstChar = STB_FONT_consolas_24_latin1_FIRST_CHAR;

    auto & data = _vertexTracker->Data();
    auto * mapped = &data.data[_letterCount * 4];
    
    const float charW = 1.5f * params.scale / windowWidth;
    const float charH = 1.5f * params.scale / windowHeight;

    float fbW = windowWidth;
    float fbH = windowHeight;
    x = (x / fbW * 2.0f) - 1.0f;
    y = (y / fbH * 2.0f) - 1.0f;

    // Calculate text width
    float textWidth = 0;
    for (auto letter : text)
    {
        stb_fontchar *charData = &_stbFontData[(uint32_t)letter - firstChar];
        textWidth += charData->advance * charW;
    }

    switch (params.textAlign)
    {
        case TextAlign::Right:
            x -= textWidth;
            break;
        case TextAlign::Center:
            x -= textWidth / 2.0f;
            break;
        case TextAlign::Left:
            break;
    }

    // Generate a uv mapped quad per char in the new text
    for (auto letter : text)
    {
        stb_fontchar *charData = &_stbFontData[(uint32_t)letter - firstChar];

        mapped->position.x = (x + (float)charData->x0 * charW);
        mapped->position.y = (y + (float)charData->y0 * charH);
        mapped->uv.x = charData->s0;
        mapped->uv.y = charData->t0;
        mapped++;

        mapped->position.x = (x + (float)charData->x1 * charW);
        mapped->position.y = (y + (float)charData->y0 * charH);
        mapped->uv.x = charData->s1;
        mapped->uv.y = charData->t0;
        mapped++;

        mapped->position.x = (x + (float)charData->x0 * charW);
        mapped->position.y = (y + (float)charData->y1 * charH);
        mapped->uv.x = charData->s0;
        mapped->uv.y = charData->t1;
        mapped++;

        mapped->position.x = (x + (float)charData->x1 * charW);
        mapped->position.y = (y + (float)charData->y1 * charH);
        mapped->uv.x = charData->s1;
        mapped->uv.y = charData->t1;
        mapped++;

        x += charData->advance * charW;

        _letterCount++;
    }
}

//------------------------------------------------------------------
