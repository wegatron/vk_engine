#include <framework/vk/syncs.h>

namespace vk_engine
{
    Fence::Fence(const std::shared_ptr<VkDriver> &driver, const bool signaled) : driver_(driver)
    {
        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

        if (vkCreateFence(driver_->getDevice(), &fenceCreateInfo, nullptr, &handle_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create fence!");
        }
    }

    Fence::~Fence()
    {
        vkDestroyFence(driver_->getDevice(), handle_, nullptr); 
    }

    /**
     * @brief Reset the fence to unsignaled state
    */
    void Fence::reset()
    {        
        vkResetFences(driver_->getDevice(), 1, &handle_);
    }

    void Fence::wait(const uint64_t timeout = std::numeric_limits<uint64_t>::max()) const
    {        
        vkWaitForFences(driver_->getDevice(), 1, &handle_, VK_TRUE, timeout);
    }

    VkResult Fence::getStatus() const
    {
        return vkGetFenceStatus(driver_->getDevice(), handle_);
    }

    Semaphore::Semaphore(const std::shared_ptr<VkDriver> &driver)
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(driver->getDevice(), &semaphoreCreateInfo, nullptr, &handle_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphore!");
        }
    }

    Semaphore::~Semaphore()
    {
        vkDestroySemaphore(driver_->getDevice(), handle_, nullptr);
    }
}
