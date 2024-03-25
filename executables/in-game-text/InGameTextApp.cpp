#include "InGameTextApp.hpp"

using namespace MFA;

//------------------------------------------------------------------

InGameTextApp::InGameTextApp(std::shared_ptr<SwapChainRenderResource> swapChainResource)
{
    mDevice = LogicalDevice::Instance;

    mDepthResource = std::make_shared<DepthRenderResource>();
    mMSAA_Resource = std::make_shared<MSSAA_RenderResource>();
    mDisplayRenderPass = std::make_shared<DisplayRenderPass>(
    	std::move(swapChainResource),
    	mDepthResource,
    	mMSAA_Resource
    );
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
    mDevice->BeginCommandBuffer(
    	recordState,
    	RT::CommandBufferType::Compute
    );

    mDevice->EndCommandBuffer(recordState);

    mDevice->BeginCommandBuffer(
    	recordState,
    	RT::CommandBufferType::Graphic
    );

    mDisplayRenderPass->Begin(recordState);

    mDisplayRenderPass->End(recordState);

    mDevice->EndCommandBuffer(recordState);
}

//------------------------------------------------------------------