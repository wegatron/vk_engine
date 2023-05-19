#include "shader_module.h"
#include <cassert>
#include <fstream>
#include "glslang/Public/ShaderLang.h"
#include "glslang/Public/ResourceLimits.h"

namespace vk_engine
{
    void ShaderSource::load(const std::string &filepath)
    {
        auto len = filepath.length();
        assert(len > 4);
        std::ifstream ifs(filepath, std::ifstream::binary);
        if(!ifs) throw std::runtime_error("can't open file " + filepath);
        ifs.seekg(0, std::ios::end);
        size_t size = ifs.tellg();

        if( 0 == filepath.compare(len-4, 4, ".spv"))
        {
            if(0 == filepath.compare(len-9, 9, ".vert.spv"))
            {
                stage_ = VK_SHADER_STAGE_VERTEX_BIT;
            } else if(0 == filepath.compare(len-9, 9, ".frag.spv"))
            {
                stage_ = VK_SHADER_STAGE_FRAGMENT_BIT;
            } else if(0 == filepath.compare(len-9, 9, ".comp.vert"))
            {
                stage_ = VK_SHADER_STAGE_COMPUTE_BIT;
            }
            assert(size%4==0);
            spirv_code_.resize(size/4);
            ifs.read(reinterpret_cast<char*>(spirv_code_.data()), size);            
        } else {
            // glslcode compile to spirv code
            glsl_code_.resize(size);
            ifs.read(reinterpret_cast<char*>(glsl_code_.data()), size);
            compile2spirv();
        }
        ifs.close();

        // update hash code
        std::hash<std::string> hasher{};
        hash_code_ = hasher(std::string{
            reinterpret_cast<const char *>(spirv_code_.data(),
            reinterpret_cast<const char *>(spirv_code_.data() + spirv_code_.size()))});        
    }

    EShLanguage findShaderLanguage(VkShaderStageFlagBits stage);

    void ShaderSource::compile2spirv()
    {
    	// Initialize glslang library.
        glslang::InitializeProcess();

        // TODO add support for shader varient

        EShLanguage lang = findShaderLanguage(stage_);
        glslang::TShader shader(lang);
        const char *file_name_list[1] = {""};
        const char * shader_source = glsl_code_.data();
        shader.setStringsWithLengthsAndNames(&shader_source, nullptr, file_name_list, 1);
        shader.setEntryPoint("main");
        shader.setSourceEntryPoint("main");
        EShMessages messages = static_cast<EShMessages>(EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules);

        if (!shader.parse(GetDefaultResources(), 100, false, messages))
        {
            auto error_msg = std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog());
            throw std::runtime_error("compile glsl to spirv error!"+error_msg);
        }        
        
    }



    // helper functions
    EShLanguage findShaderLanguage(VkShaderStageFlagBits stage)
    {
        switch (stage)
        {
            case VK_SHADER_STAGE_VERTEX_BIT:
                return EShLangVertex;

            case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
                return EShLangTessControl;

            case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
                return EShLangTessEvaluation;

            case VK_SHADER_STAGE_GEOMETRY_BIT:
                return EShLangGeometry;

            case VK_SHADER_STAGE_FRAGMENT_BIT:
                return EShLangFragment;

            case VK_SHADER_STAGE_COMPUTE_BIT:
                return EShLangCompute;

            case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
                return EShLangRayGen;

            case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
                return EShLangAnyHit;

            case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
                return EShLangClosestHit;

            case VK_SHADER_STAGE_MISS_BIT_KHR:
                return EShLangMiss;

            case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
                return EShLangIntersect;

            case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
                return EShLangCallable;

            default:
                return EShLangVertex;
        }
    }
}