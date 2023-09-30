#pragma once

#include "LogicalDevice.hpp"
#include "RenderBackend.hpp"
#include "RenderTypes.hpp"

namespace MFA
{
    template<typename T>
    struct HostVisibleBufferTracker
    {
    public:

        explicit HostVisibleBufferTracker(std::shared_ptr<RT::BufferGroup> bufferGroup, T data)
            : mBufferGroup(std::move(bufferGroup))
            , mData(data)
        {
            
            for (auto const& buffer : mBufferGroup->buffers)
            {
                RB::UpdateHostVisibleBuffer(
                    LogicalDevice::Instance->GetVkDevice(),
                    *buffer,
                    Alias(mData)
                );
            }
            mDirtyCounter = 0;
        }

        void Update(RT::CommandRecordState const & recordState)
        {
            if (mDirtyCounter > 0)
            {
                RB::UpdateHostVisibleBuffer(
                    LogicalDevice::Instance->GetVkDevice(),
                    *mBufferGroup->buffers[recordState.frameIndex],
                    Alias(mData)
                );
                --mDirtyCounter;
            }
        }

        void SetData(T data)
        {
            mData = data;
            mDirtyCounter = LogicalDevice::Instance->GetMaxFramePerFlight();
        }

    private:
        
        std::shared_ptr<RT::BufferGroup> mBufferGroup;
        int mDirtyCounter = 0;
        T mData {};
    };

    // TODO: We might need a localBufferTracker as well
}