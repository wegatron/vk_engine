file(GLOB src *.cpp)
file(GLOB headers *.h)
add_executable(vk_triangle
    ${src}
    ${headers})

target_link_libraries(vk_triangle 
    PRIVATE 
    vk_engine
    volk
    ${GLFW_LIBRARY}
    ${THREAD} 
    glslang::glslang    
    glslang::glslang-default-resource-limits
    glslang::SPIRV
    VulkanMemoryAllocator::VulkanMemoryAllocator
    spirv-cross-reflect
    spirv-cross-core
    spirv-cross-glsl
)