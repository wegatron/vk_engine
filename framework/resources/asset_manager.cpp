#include <framework/resources/asset_manager.hpp>

#include <stb_image.h>
#include <cassert>
#include <framework/utils/base/data_reshaper.hpp>
#include <framework/functional/global/app_context.h>
#include <framework/utils/vk/image.h>
#include <framework/utils/vk/vk_driver.h>
#include <framework/utils/vk/commands.h>


namespace vk_engine
{
    static constexpr uint32_t ASSET_TIME_BEFORE_EVICTION = 100;   

    std::shared_ptr<ImageView> createImageView(void * img_data, uint32_t width, uint32_t height, uint32_t channel,
        const std::shared_ptr<CommandBuffer> &cmd_buf)
    {
        VkExtent3D extent{width, height, 1};
        
        // if channel is not 4 need to add channel to it
        void *data_ptr = img_data;
        if (channel != 4) {
            data_ptr = new uint8_t[width * height * 4];
            reshapeImageData<uint8_t>(data_ptr, img_data, 3, 4,
                                      width * height * channel);
        }

        auto driver = getDefaultAppContext().driver;
        auto image = std::make_shared<Image>(
            driver, 0, VK_FORMAT_R8G8B8A8_SRGB, extent, VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        image->updateByStaging(data_ptr, getDefaultAppContext().stage_pool, cmd_buf);
        if (data_ptr != img_data) delete[] static_cast<uint8_t *>(data_ptr);
        VkImageSubresourceRange range = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };        
        image->transitionLayout(cmd_buf->getHandle(), range,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        auto img_v = std::make_shared<ImageView>(
            image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 1);

        return img_v;
    }

    std::shared_ptr<ImageView> createImageView(const float * img_data, uint32_t width, uint32_t height, uint32_t channel,
        const std::shared_ptr<CommandBuffer> &cmd_buf)
    {
        VkExtent3D extent{width, height, 1};
        assert(channel == 4);
        auto driver = getDefaultAppContext().driver;
        auto image = std::make_shared<Image>(
            driver, 0, VK_FORMAT_R32G32B32A32_SFLOAT, extent, VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        void * data_ptr = const_cast<float*>(img_data);
        image->updateByStaging(data_ptr, getDefaultAppContext().stage_pool, cmd_buf);
        VkImageSubresourceRange range = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };
        image->transitionLayout(cmd_buf->getHandle(), range,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        auto img_v = std::make_shared<ImageView>(
            image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 1);
        return img_v;      
    }

    template <>
    std::shared_ptr<ImageView> load(const std::string &path, const std::shared_ptr<CommandBuffer> &cmd_buf) {
        int width = 0;
        int height = 0;
        int channel = 0;
        stbi_uc *img_data = stbi_load(path.c_str(), &width, &height, &channel, 0);
        auto ret = createImageView(img_data, width, height, channel, cmd_buf);
        stbi_image_free(img_data);
        return ret;
    }

    template <>
    std::shared_ptr<ImageView> load(const uint8_t *data, const size_t size, const std::shared_ptr<CommandBuffer> &cmd_buf)
    {
        int width = 0;
        int height = 0;
        int channel = 0;
        stbi_uc *img_data = stbi_load_from_memory(data, size, &width, &height, &channel, 0);
        auto ret = createImageView(img_data, width, height, channel, cmd_buf);        
        stbi_image_free(img_data);        
        return ret;
    }

    template <> std::shared_ptr<ImageView> load(const float *data, const uint32_t width, const uint32_t height, const uint32_t channel,
        const std::shared_ptr<CommandBuffer> &cmd_buf)
    {
        auto ret = createImageView(data, width, height, channel, cmd_buf);
        return ret;
    } 

    void GPUAssetManager::gc()
    {
        if(++current_frame_ < ASSET_TIME_BEFORE_EVICTION) return;
        for(auto itr = assets_.begin(); itr != assets_.end();)
        {
            auto & a = itr->second;
            if(a.data_ptr.use_count() == 1 && a.last_accessed + ASSET_TIME_BEFORE_EVICTION <= current_frame_)
            {
                itr = assets_.erase(itr);
            } else ++itr;
        }
    }

    void GPUAssetManager::reset()
    {        
        assets_.clear();
    }
}