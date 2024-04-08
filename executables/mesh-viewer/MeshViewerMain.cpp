//#include <thread>
//#include <omp.h>

#include "BedrockLog.hpp"
#include "BedrockPath.hpp"
#include "LogicalDevice.hpp"
#include "pipeline/LinePipeline.hpp"
#include "render_pass/DisplayRenderPass.hpp"
#include "render_resource/DepthRenderResource.hpp"
#include "render_resource/MSAA_RenderResource.hpp"
#include "render_resource/SwapChainRenderResource.hpp"
#include "BufferTracker.hpp"
#include "camera/ArcballCamera.hpp"
#include "UI.hpp"
#include "pipeline/FlatShadingPipeline.hpp"
#include "ImportTexture.hpp"
#include "ImportGLTF.hpp"
#include "camera/ObserverCamera.hpp"
#include "utils/PointRenderer.hpp"
#include "utils/MeshRenderer.hpp"
#include "utils/LineRenderer.hpp"

#include <future>
#include <glm/glm.hpp>


using namespace MFA;

bool displayWireframe = false;

void UI_Loop()
{
	auto ui = MFA::UI::Instance;

	ui->BeginWindow("Settings");
	ImGui::Checkbox("Display wireframe", &displayWireframe);
	ui->EndWindow();
};

//-------------------------------------------------------------------------------------------------

std::shared_ptr<RT::GpuTexture> CreateErrorTexture()
{
	auto const* device = LogicalDevice::Instance;

	auto const commandBuffer = RB::BeginSingleTimeCommand(
		device->GetVkDevice(),
		device->GetGraphicCommandPool()
	);

	auto const cpuTexture = Importer::ErrorTexture();

	auto [gpuTexture, stagingBuffer] = RB::CreateTexture(
		*cpuTexture,
		LogicalDevice::Instance->GetVkDevice(),
		LogicalDevice::Instance->GetPhysicalDevice(),
		commandBuffer
	);

	RB::EndAndSubmitSingleTimeCommand(
		device->GetVkDevice(),
		device->GetGraphicCommandPool(),
		device->GetGraphicQueue(),
		commandBuffer
	);

	return gpuTexture;
}

//-------------------------------------------------------------------------------------------------

int main()
{

	MFA_LOG_DEBUG("Loading...");

	//auto threadCount = std::thread::hardware_concurrency() / 2;
	//omp_set_num_threads(threadCount);
	//MFA_LOG_INFO("Thread count is %d", omp_get_max_threads());

	auto path = Path::Instantiate();
	
	LogicalDevice::InitParams params
	{
		.windowWidth = 1000,
		.windowHeight = 1000,
		.resizable = true,
		.fullScreen = false,
		.applicationName = "Mesh-Viewer-App"
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

		auto const ui = std::make_shared<UI>(displayRenderPass);

		auto cameraBuffer = RB::CreateHostVisibleUniformBuffer(
			device->GetVkDevice(),
			device->GetPhysicalDevice(),
			sizeof(glm::mat4),
			device->GetMaxFramePerFlight()
		);

		ArcballCamera camera{};
		camera.Setposition({ 0.0f, -1.0f, 15.0f });

		auto cameraBufferTracker = std::make_shared<HostVisibleBufferTracker>(cameraBuffer, Alias{ camera.GetViewProjection() });

		device->ResizeEventSignal2.Register([&cameraBufferTracker, &camera]()->void {
			cameraBufferTracker->SetData(Alias{ camera.GetViewProjection() });
		});

		auto defaultSampler = RB::CreateSampler(LogicalDevice::Instance->GetVkDevice(), {});

		auto const shadingPipeline1 = std::make_shared<FlatShadingPipeline>(
			displayRenderPass,
			cameraBuffer,
			defaultSampler,
			FlatShadingPipeline::Params{
				.maxSets = 100,
				.cullModeFlags = VK_CULL_MODE_BACK_BIT,
			}
		);

		auto const shadingPipeline2 = std::make_shared<FlatShadingPipeline>(
			displayRenderPass,
			cameraBuffer,
			defaultSampler,
			FlatShadingPipeline::Params{
				.maxSets = 100,
				.cullModeFlags = VK_CULL_MODE_FRONT_BIT,
			}
		);

		auto const wireFramePipeline = std::make_shared<FlatShadingPipeline>(
			displayRenderPass,
			cameraBuffer,
			defaultSampler,
			FlatShadingPipeline::Params{
				.maxSets = 100,
				.cullModeFlags = VK_CULL_MODE_NONE,
				.polygonMode = VK_POLYGON_MODE_LINE
			}
		);

		auto const pointPipeline = std::make_shared<PointPipeline>(displayRenderPass, cameraBuffer, 10000);
		auto pointRenderer = std::make_shared<PointRenderer>(pointPipeline);

		auto const linePipeline = std::make_shared<LinePipeline>(displayRenderPass, cameraBuffer, 10000);
		auto lineRenderer = std::make_shared<LineRenderer>(linePipeline);

		ui->UpdateSignal.Register([]()->void { UI_Loop(); });

		auto errorTexture = CreateErrorTexture();

		auto subMarineModel = Importer::GLTF_Model(Path::Instance->Get("models/submarine/scene.gltf"));

		auto submarineRenderer = std::make_shared<MeshRenderer>(
			shadingPipeline1,
			subMarineModel,
			errorTexture
		);

		auto submarineWireFrameRenderer = std::make_shared<MeshRenderer>(
			wireFramePipeline,
			subMarineModel,
			errorTexture
		);

		glm::mat4 submarineModelMat{};
		{
			auto const scale = glm::scale(glm::identity<glm::mat4>(), { 0.02f, 0.02f, 0.02f });
			// auto const rotation1 = glm::angleAxis(glm::radians(180.0f), MFA::Math::UpVec3);
			// auto const rotation2 = glm::angleAxis(glm::radians(-90.0f), MFA::Math::RightVec3);
			// auto const rotation3 = glm::angleAxis(glm::radians(0.0f), MFA::Math::UpVec3);
			submarineModelMat = /*glm::toMat4(rotation3) * glm::toMat4(rotation2) * glm::toMat4(rotation1) * */scale;
		}
		
		const uint32_t MinDeltaTimeMs = 1000 / 60;

		SDL_GL_SetSwapInterval(0);
		SDL_Event e;
		uint32_t deltaTimeMs = MinDeltaTimeMs;
		float deltaTimeSec = static_cast<float>(MinDeltaTimeMs) / 1000.0f;
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

			camera.Update(deltaTimeSec);
			if (camera.IsDirty())
			{
				cameraBufferTracker->SetData(Alias{ camera.GetViewProjection() });
			}

			ui->Update();

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

				displayRenderPass->Begin(recordState);

				if (displayWireframe == true)
				{
					submarineWireFrameRenderer->Render(recordState, { submarineModelMat });
				}
				else
				{
					submarineRenderer->Render(recordState, { submarineModelMat });
				}
				
				ui->Render(recordState, deltaTimeSec);

				displayRenderPass->End(recordState);
				device->EndCommandBuffer(recordState);

				device->SubmitQueues(recordState);
				device->Present(recordState, swapChainResource->GetSwapChainImages().swapChain);
			}

			deltaTimeMs = SDL_GetTicks() - startTime;
			if (MinDeltaTimeMs > deltaTimeMs)
			{
				SDL_Delay(MinDeltaTimeMs - deltaTimeMs);
			}

			deltaTimeMs = SDL_GetTicks() - startTime;
			deltaTimeSec = static_cast<float>(deltaTimeMs) / 1000.0f;
			startTime = SDL_GetTicks();
		}

		device->DeviceWaitIdle();
	}

	MFA_LOG_DEBUG("Cleanup is complete");

	return 0;
}
