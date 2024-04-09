#include "BedrockLog.hpp"
#include "BedrockPath.hpp"
#include "LogicalDevice.hpp"
#include "render_resource/SwapChainRenderResource.hpp"
#include "UI.hpp"
#include "SoundApp.hpp"

using namespace MFA;

int main()
{
    MFA_LOG_INFO("Loading!");

    auto path = Path::Instantiate();

    LogicalDevice::InitParams params{
        .windowWidth = 1000,
        .windowHeight = 1000,
        .resizable = true,
        .fullScreen = false,
        .applicationName = "In-Game-Text"
    };

    auto const device = LogicalDevice::Instantiate(params);
    if (device->IsValid() == false)
    {
        MFA_LOG_ERROR("Failed to create the device");
        return -1;
    }

    {
        auto swapChainResource = std::make_shared<SwapChainRenderResource>();

        auto app = std::make_unique<SoundApp>(swapChainResource);

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

            app->Update(deltaTimeSec);

			auto recordState = device->AcquireRecordState(swapChainResource->GetSwapChainImages().swapChain);
			if (recordState.isValid == true)
			{
                app->Render(recordState);
				
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