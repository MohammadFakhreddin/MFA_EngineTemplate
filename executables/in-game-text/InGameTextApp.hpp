#pragma once

#include "RenderTypes.hpp"
#include "LogicalDevice.hpp"
#include "render_resource/SwapChainRenderResource.hpp"
#include "render_pass/DisplayRenderPass.hpp"
#include "render_resource/DepthRenderResource.hpp"
#include "render_resource/MSAA_RenderResource.hpp"
#include "BufferTracker.hpp"
#include "utils/ConsolasFontRenderer.hpp"

class InGameTextApp
{
public:

    explicit InGameTextApp(std::shared_ptr<MFA::SwapChainRenderResource> swapChainResource);

    ~InGameTextApp();

    void Update(float deltaTimeSec);

    void Render(MFA::RT::CommandRecordState & recordState);

private:

    void CreateFontSampler();

    MFA::LogicalDevice * _device = nullptr;

    std::shared_ptr<MFA::DepthRenderResource> _depthResource{};
    
    std::shared_ptr<MFA::MSSAA_RenderResource> _msaa_Resource{};

    std::shared_ptr<MFA::DisplayRenderPass> _displayRenderPass{};

    std::unique_ptr<MFA::ConsolasFontRenderer> _fontRenderer{};

    std::shared_ptr<MFA::RT::SamplerGroup> _fontSampler{};

    std::unique_ptr<MFA::ConsolasFontRenderer::TextData> _textData{};

};