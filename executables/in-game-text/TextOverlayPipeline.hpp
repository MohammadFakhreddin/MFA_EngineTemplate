#pragma once

#include "render_pass/DisplayRenderPass.hpp"

#include <glm/glm.hpp>

class TextOverlayPipeline
{
public:

    struct Vertex
    {
        glm::vec2 position{};
        glm::vec2 uv{};
    };

    explicit TextOverlayPipeline(
        std::shared_ptr<MFA::DisplayRenderPass> displayRenderPass,
        std::shared_ptr<MFA::RT::SamplerGroup> sampler
    );

private:

    void CreateDescriptorLayout();
    
    MFA::RT::DescriptorSetGroup CreateDescriptorSet(
        MFA::RT::SamplerGroup const & sampler, 
        MFA::RT::GpuTexture const & texture
    );

    void CreatePipeline();
    
    std::shared_ptr<MFA::DisplayRenderPass> _displayRenderPass{};
    std::shared_ptr<MFA::RT::SamplerGroup> _sampler{};

    std::shared_ptr<MFA::RT::DescriptorPool> _descriptorPool{};

    std::shared_ptr<MFA::RT::DescriptorSetLayoutGroup> _descriptorLayout{};

};