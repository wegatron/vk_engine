#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <volk.h>
#include <framework/vk/descriptor_set_layout.h>


namespace vk_engine {
VkDescriptorType find_descriptor_type(ShaderResourceType resource_type,
                                      bool dynamic) {
  switch (resource_type) {
  case ShaderResourceType::InputAttachment:
    return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    break;
  case ShaderResourceType::Image:
    // an image that can be used in conjunction with a sampler to provide
    // filtered data to a shader.
    return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    break;
  case ShaderResourceType::ImageSampler:
    // a sampler and an image paired together, can be more efficient on some
    // architectures
    return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    break;
  case ShaderResourceType::ImageStorage:
    // image that cannot be used with a sampler but can be written to
    return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    break;
  case ShaderResourceType::Sampler:
    return VK_DESCRIPTOR_TYPE_SAMPLER;
    break;
  case ShaderResourceType::BufferUniform:
    // dynamic, include an offset and size that are passed when the descriptor
    // set is bound to the pipeline can use the same descriptor set for multiple
    // objects with different offsets refer to:
    // https://github.com/SaschaWillems/Vulkan/blob/master/examples/dynamicuniformbuffer/README.md
    if (dynamic) {
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    } else {
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }
    break;
  case ShaderResourceType::BufferStorage:
    if (dynamic) {
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    } else {
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    }
    break;
  default:
    throw std::runtime_error(
        "No conversion possible for the shader resource type.");
    break;
  }
}

DescriptorSetLayout::DescriptorSetLayout(
    const std::shared_ptr<VkDriver> &driver,
    const uint32_t set_index,
    const std::vector<ShaderResource> &resource_set) 
      : driver_(driver), set_index_(set_index) {
  for (auto &resource : resource_set) {
    if (resource.type == ShaderResourceType::Input ||
        resource.type == ShaderResourceType::Output ||
        resource.type == ShaderResourceType::PushConstant ||
        resource.type == ShaderResourceType::SpecializationConstant)
      continue;

    auto descriptor_type = find_descriptor_type(
        resource.type, resource.mode == ShaderResourceMode::Dynamic);

    if (resource.mode == ShaderResourceMode::UpdateAfterBind)
      binding_flags_.push_back(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT);
    else
      binding_flags_.push_back(0);

    VkDescriptorSetLayoutBinding layout_binding{};
    layout_binding.binding = resource.binding;
    layout_binding.descriptorType = descriptor_type;
    layout_binding.descriptorCount = resource.array_size;
    layout_binding.stageFlags = resource.stages;
    bindings_.push_back(layout_binding);
  }

  VkDescriptorSetLayoutCreateInfo layout_info{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  layout_info.bindingCount = static_cast<uint32_t>(bindings_.size());
  layout_info.pBindings = bindings_.data();

  // handle update after bind extensions
  if (std::find_if(resource_set.begin(), resource_set.end(),
                   [](const ShaderResource &resource) {
                     return resource.mode ==
                            ShaderResourceMode::UpdateAfterBind;
                   }) != resource_set.end()) {

    // Spec states you can't have ANY dynamic resources if you have one of the
    // bindings set to update-after-bind
    if (std::find_if(resource_set.begin(), resource_set.end(),
                     [](const ShaderResource &shader_resource) {
                       return shader_resource.mode ==
                              ShaderResourceMode::Dynamic;
                     }) != resource_set.end()) {
      throw std::runtime_error("Cannot create descriptor set layout, \
                    dynamic resources are not allowed if at least one resource is update-after-bind.");
    }

    assert(bindings_.size() == binding_flags_.size());

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT binding_flags_info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT};
    binding_flags_info.bindingCount =
        static_cast<uint32_t>(binding_flags_.size());
    binding_flags_info.pBindingFlags = binding_flags_.data();

    layout_info.pNext = &binding_flags_info;
    layout_info.flags =
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
  }

  VkResult result = vkCreateDescriptorSetLayout(
      driver->getDevice(), &layout_info, nullptr, &handle_);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor set layout.");
  }
}

DescriptorSetLayout::~DescriptorSetLayout() {
  if (handle_ != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(driver_->getDevice(), handle_, nullptr);
    handle_ = VK_NULL_HANDLE;
  }
}
} // namespace vk_engine