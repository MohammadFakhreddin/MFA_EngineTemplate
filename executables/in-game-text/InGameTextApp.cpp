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

    PrepareResources();
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

void InGameTextApp::PrepareResources()
{
    auto const fontWidth = STB_FONT_consolas_24_latin1_BITMAP_WIDTH;
    auto const fontHeight = STB_FONT_consolas_24_latin1_BITMAP_HEIGHT;

    uint8_t font24pixels[fontHeight][fontWidth];
    stb_font_consolas_24_latin1(_stbFontData, font24pixels, fontHeight);

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
