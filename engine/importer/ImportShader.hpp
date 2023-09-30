#pragma once

#include <memory>
#include <string>

#include "AssetShader.hpp"
#include "BedrockMemory.hpp"

namespace MFA::Importer
{

    std::shared_ptr<AS::Shader> ShaderFromSPV(
        std::string const & path,
        VkShaderStageFlagBits const stage,
        std::string const & entryPoint
    );

    std::shared_ptr<AS::Shader> ShaderFromSPV(
        BaseBlob const & dataMemory,
        VkShaderStageFlagBits stage,
        std::string const & entryPoint
    );
}
