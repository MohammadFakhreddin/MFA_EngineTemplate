#include "BedrockLog.hpp"
#include "BedrockPath.hpp"

#include "LogicalDevice.hpp"
#include "pipeline/LinePipeline.hpp"
#include "render_pass/DisplayRenderPass.hpp"
#include "render_resource/DepthRenderResource.hpp"
#include "render_resource/MSAA_RenderResource.hpp"
#include "render_resource/SwapChainRenderResource.hpp"
#include "BedrockMath.hpp"
#include "BufferTracker.hpp"

#include <glm/glm.hpp>

#include "camera/PerspectiveCamera.hpp"

int main()
{
    using namespace MFA;

    MFA_LOG_DEBUG("Loading...");

    auto path = Path::Instantiate();

    LogicalDevice::InitParams params
    {
        .windowWidth = 800,
        .windowHeight = 600,
        .resizable = true,
        .fullScreen = false,
        .applicationName = "PBD"
    };

    auto const device = LogicalDevice::Instantiate(params);
    assert(device->IsValid() == true);

    {

        auto swapChainResource = std::make_shared<SwapChainRenderResource>();
        auto depthResource = std::make_shared<DepthRenderResource>();
        auto msaaResource = std::make_shared<MSSAA_RenderResource>();
        auto displayRenderPass = std::make_shared<DisplayRenderPass>(
            swapChainResource,
            depthResource,
            msaaResource
        );

        auto cameraBuffer = RB::CreateHostVisibleUniformBuffer(
            device->GetVkDevice(),
            device->GetPhysicalDevice(),
            sizeof(glm::mat4),
            device->GetMaxFramePerFlight()
        );

        PerspectiveCamera camera{};

        auto ComputeViewProjectionMat4 = [&camera]()->glm::mat4
        {
            return camera.GetViewProjection();
        };

        auto cameraBufferTracker = std::make_shared<HostVisibleBufferTracker<glm::mat4>>(cameraBuffer, ComputeViewProjectionMat4());

        device->ResizeEventSignal2.Register([&cameraBufferTracker, &ComputeViewProjectionMat4]()->void {
            cameraBufferTracker->SetData(ComputeViewProjectionMat4());
        });

        auto const linePipeline = std::make_shared<LinePipeline>(displayRenderPass, cameraBuffer, 10000);

        std::vector<glm::vec3> vertices{
            glm::vec3 {0.5f, 0.5f, 5.0f},
            glm::vec3 {-0.5f, -0.5f, 5.0f},
        };
        std::vector<uint16_t> indices{ 0, 1 };

        Alias const verticesAlias{ vertices.data(), vertices.size() };
        Alias const indicesAlias{ indices.data(), indices.size() };

        auto commandBuffer = RB::BeginSingleTimeCommand(
            device->GetVkDevice(),
            device->GetGraphicCommandPool()
        );
        auto vertexStageBuffer = RB::CreateStageBuffer(
            device->GetVkDevice(),
            device->GetPhysicalDevice(),
            verticesAlias.Len(),
            1
        );
        auto vertexBuffer = RB::CreateVertexBuffer(
            device->GetVkDevice(),
            device->GetPhysicalDevice(),
            commandBuffer,
            *vertexStageBuffer->buffers[0],
            verticesAlias
        );
        auto indexStageBuffer = RB::CreateStageBuffer(
            device->GetVkDevice(),
            device->GetPhysicalDevice(),
            indicesAlias.Len(),
            1
        );
        auto indexBuffer = RB::CreateIndexBuffer(
            device->GetVkDevice(),
            device->GetPhysicalDevice(),
            commandBuffer,
            *indexStageBuffer->buffers[0],
            indicesAlias
        );
        RB::EndAndSubmitSingleTimeCommand(
            device->GetVkDevice(),
            device->GetGraphicCommandPool(),
            device->GetGraphicQueue(),
            commandBuffer
        );

        SDL_GL_SetSwapInterval(0);
        SDL_Event e;
        uint32_t deltaTimeMs = 0;
        float deltaTimeSec = 0.0f;
        uint32_t startTime = SDL_GetTicks();

        bool shouldQuit = false;

        while (shouldQuit == false)
        {
            //Handle events
            while (SDL_PollEvent(&e) != 0)
            {
                //User requests quit
                if (e.type == SDL_QUIT)
                {
                    shouldQuit = true;
                }
            }

            device->Update();

            auto recordState = device->AcquireRecordState(swapChainResource->GetSwapChainImages().swapChain);
            if (recordState.isValid == true)
            {
                device->BeginCommandBuffer(
                    recordState,
                    RT::CommandBufferType::Compute
                );
                device->EndCommandBuffer(recordState);

                device->BeginCommandBuffer(
                    recordState,
                    RT::CommandBufferType::Graphic
                );

                cameraBufferTracker->Update(recordState);

                vkCmdSetLineWidth(
                    recordState.commandBuffer,
                    100
                );

                displayRenderPass->Begin(recordState);

                linePipeline->BindPipeline(recordState);

                linePipeline->SetPushConstants(
                    recordState,
                    LinePipeline::PushConstants{
                        .model = glm::identity<glm::mat4>(),
                        .color = glm::vec4 {1.0f, 0.0f, 0.0f, 1.0f}
                    }
                );

                RB::BindIndexBuffer(
                    recordState,
                    *indexBuffer,
                    0,
                    VK_INDEX_TYPE_UINT16
                );

                RB::BindVertexBuffer(
                    recordState,
                    *vertexBuffer,
                    0,
                    0
                );

                RB::DrawIndexed(
                    recordState,
                    indices.size()
                );

                displayRenderPass->End(recordState);
                device->EndCommandBuffer(recordState);

                device->SubmitQueues(recordState);
                device->Present(recordState, swapChainResource->GetSwapChainImages().swapChain);
            }

            deltaTimeMs = SDL_GetTicks() - startTime;
            startTime = SDL_GetTicks();
            deltaTimeSec = static_cast<float>(deltaTimeMs) / 1000.0f;
        }

        device->DeviceWaitIdle();
    }

    MFA_LOG_DEBUG("Cleanup is complete");

    return 0;
}
