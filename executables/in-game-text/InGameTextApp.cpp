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
}

//------------------------------------------------------------------

InGameTextApp::~InGameTextApp()
{

}

//------------------------------------------------------------------

void InGameTextApp::Update(float deltaTimeSec)
{

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

    _displayRenderPass->Begin(recordState);

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
