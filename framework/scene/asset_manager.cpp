#include <framework/scene/asset_manager.hpp>

#include <stb_image.h>
#include <cassert>
#include <framework/utils/data_reshaper.hpp>
#include <framework/utils/app_context.h>
#include <framework/vk/image.h>
#include <framework/vk/vk_driver.h>
#include <framework/vk/commands.h>


namespace vk_engine
{
    static constexpr uint32_t ASSET_TIME_BEFORE_EVICTION = 100;   

    std::shared_ptr<ImageView> createImageView(void * img_data, int width, int height, int channel,
        const std::shared_ptr<CommandBuffer> &cmd_buf)
    {
        VkExtent3D extent{static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
        
        // if channel is not 4 need to add channel to it
        void *data_ptr = img_data;
        if (channel != 4) {
            data_ptr = new uint8_t[width * height * 4];            
            reshapeImageData<uint8_t>(img_data, data_ptr,
                                      width * height * channel, channel, 4);
        }

        auto driver = getDefaultAppContext().driver;
        auto image = std::make_shared<Image>(
            driver, 0, VK_FORMAT_R8G8B8A8_SRGB, extent, VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        image->updateByStaging(data_ptr, getDefaultAppContext().stage_pool, cmd_buf);
        
        // add memory barrier
        ImageMemoryBarrier barrier{
        .old_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .new_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .src_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
        .dst_queue_family_index = VK_QUEUE_FAMILY_IGNORED};        
        cmd_buf->imageMemoryBarrier();
        if (data_ptr != img_data)
          delete[] static_cast<uint8_t *>(data_ptr);
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