#pragma once

#include "RenderTypes.hpp"
#include "LogicalDevice.hpp"
#include "render_resource/SwapChainRenderResource.hpp"
#include "render_pass/DisplayRenderPass.hpp"
#include "render_resource/DepthRenderResource.hpp"
#include "render_resource/MSAA_RenderResource.hpp"
#include "stb_font_consolas_24_latin1.h"
#include "BufferTracker.hpp"
#include "TextOverlayPipeline.hpp"

#include <glm/glm.hpp>

class InGameTextApp
{
public:

    explicit InGameTextApp(std::shared_ptr<MFA::SwapChainRenderResource> swapChainResource);

    ~InGameTextApp();

    void Update(float deltaTimeSec);

    void Render(MFA::RT::CommandRecordState & recordState);

private:

    void CreateTextVertexBuffer();

    void CreateFontTextureBuffer();

    void CreateFontSampler();

    enum class TextAlign {Center, Left, Right};
    
    struct AddTextParams
    {
        TextAlign textAlign = TextAlign::Left;
        float scale = 1.5f;
    };
    void AddText(
        std::string_view const & text, 
        float x, 
        float y, 
        AddTextParams params
    );

    MFA::LogicalDevice * _device = nullptr;

    std::shared_ptr<MFA::DepthRenderResource> _depthResource{};
    
    std::shared_ptr<MFA::MSSAA_RenderResource> _msaa_Resource{};

    std::shared_ptr<MFA::DisplayRenderPass> _displayRenderPass{};

    // Max. number of chars the text overlay buffer can hold
    static constexpr int TEXTOVERLAY_MAX_CHAR_COUNT = 2048;
    
    struct VertexData
    {
        TextOverlayPipeline::Vertex data[TEXTOVERLAY_MAX_CHAR_COUNT]{};
    };

    stb_fontchar _stbFontData[STB_FONT_consolas_24_latin1_NUM_CHARS]{};
    
    std::unique_ptr<MFA::LocalBufferTracker<VertexData>> _vertexTracker{};

    std::shared_ptr<MFA::RT::GpuTexture> _fontTexture{};
    
    std::shared_ptr<MFA::RT::SamplerGroup> _fontSampler{};

    std::unique_ptr<TextOverlayPipeline> _pipeline{};

    MFA::RT::DescriptorSetGroup _descriptorSet{};

    int _letterCount{};

};