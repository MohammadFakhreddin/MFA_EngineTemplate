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

    // Only use it for the data that is frequently updated
    template<typename T>
    struct LocalBufferTracker
    {
    public:

        explicit LocalBufferTracker(
            std::shared_ptr<RT::BufferGroup> localBuffer,
            std::shared_ptr<RT::BufferGroup> hostVisibleBuffer,
            T data 
        )
            : mLocalBuffer(std::move(localBuffer))
            , mHostVisibleBuffer(hostVisibleBuffer)
        {
            SetData(data);
        }

        void Update(RT::CommandRecordState const & recordState)
        {
            if (mDirtyCounter > 0)
            {
                RB::UpdateHostVisibleBuffer(
                    LogicalDevice::Instance->GetVkDevice(),
                    *mHostVisibleBuffer->buffers[recordState.frameIndex],
                    Alias(mData)
                );
                RB::UpdateLocalBuffer(
                    recordState.commandBuffer,
                    *mLocalBuffer->buffers[recordState.frameIndex],
                    *mHostVisibleBuffer->buffers[recordState.frameIndex]
                );
                --mDirtyCounter;
            }
        }

        void SetData(T data)
        {
            mData = data;
            mDirtyCounter = LogicalDevice::Instance->GetMaxFramePerFlight();
        }

        [[nodiscard]]
        T & Data()
        {
            // Returns the data and resets the counter.
            mDirtyCounter = LogicalDevice::Instance->GetMaxFramePerFlight();
            return mData;
        }

        RT::BufferGroup const & LocalBuffer() const
        {
            return *mLocalBuffer;
        }

    private:

        std::shared_ptr<RT::BufferGroup> mLocalBuffer{};
        std::shared_ptr<RT::BufferGroup> mHostVisibleBuffer{};
        int mDirtyCounter = 0;
        T mData{};
    };
}