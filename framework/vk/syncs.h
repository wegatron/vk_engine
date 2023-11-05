#pragma once

#include <memory>
#include <framework/vk/vk_driver.h>

namespace vk_engine
{
    /**
     * @brief Vulkan Fence, used for gpu-to-cpu synchronization
    */
    class Fence final
    {
    public:
        Fence(const std::shared_ptr<VkDriver> &driver, const bool signaled = false);
        
        Fence(const Fence &) = delete;
        Fence(Fence &&) = delete;
        Fence &operator=(const Fence &) = delete;
        Fence &operator=(Fence &&) = delete;

        ~Fence();
        
        /**
         * @brief Reset the fence to unsignaled state
        */
        void reset();

        VkResult getStatus() const;

        void wait(const uint64_t timeout = std::numeric_limits<uint64_t>::max()) const;

        VkFence getHandle() const { return handle_; }
    private:
        std::shared_ptr<VkDriver> driver_;
        VkFence handle_{VK_NULL_HANDLE};
    };


    class Semaphore final
    {
    public:
        Semaphore(const std::shared_ptr<VkDriver> &driver);
        
        Semaphore(const Semaphore &) = delete;
        Semaphore(Semaphore &&) = delete;
        Semaphore &operator=(const Semaphore &) = delete;
        Semaphore &operator=(Semaphore &&) = delete;

        ~Semaphore();
        
        VkSemaphore getHandle() const { return handle_; }

    private:
        std::shared_ptr<VkDriver> driver_;
        VkSemaphore handle_{VK_NULL_HANDLE};
    };
    

    struct RenderOutputSync {
    std::shared_ptr<Fence> render_fence;
    std::shared_ptr<Semaphore> render_semaphore;
    std::shared_ptr<Semaphore> gui_semaphore;
    std::shared_ptr<Semaphore> present_semaphore;
    };    
}