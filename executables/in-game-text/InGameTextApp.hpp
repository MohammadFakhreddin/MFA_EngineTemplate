#pragma once

#include "RenderTypes.hpp"
#include "LogicalDevice.hpp"
#include "render_resource/SwapChainRenderResource.hpp"
#include "render_pass/DisplayRenderPass.hpp"
#include "render_resource/DepthRenderResource.hpp"
#include "render_resource/MSAA_RenderResource.hpp"

class InGameTextApp
{
public:

    explicit InGameTextApp(std::shared_ptr<MFA::SwapChainRenderResource> swapChainResource);

    ~InGameTextApp();

    void Update(float deltaTimeSec);

    void Render(MFA::RT::CommandRecordState & recordState);

private:

    MFA::LogicalDevice * mDevice = nullptr;

    std::shared_ptr<MFA::DepthRenderResource> mDepthResource{};
    std::shared_ptr<MFA::MSSAA_RenderResource> mMSAA_Resource{};
    std::shared_ptr<MFA::DisplayRenderPass> mDisplayRenderPass{};
};