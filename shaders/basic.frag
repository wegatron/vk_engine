#version 450

layout(set=MATERIAL_SET_INDEX, binding = 0) uniform BasicMaterial
{
    vec4 base_color;
}pbr_mat;

#ifdef HAS_BASE_COLOR_TEXTURE
layout(set=MATERIAL_SET_INDEX, binding = 1) uniform sampler2D base_color_tex;
#endif

#ifdef HAS_METALLIC_TEXTURE
layout(set=MATERIAL_SET_INDEX, binding = 2) uniform sampler2D metallic_tex;
#endif

#ifdef HAS_SPECULAR_TEXTURE
layout(set=MATERIAL_SET_INDEX, binding = 3) uniform sampler2D specular_tex;
#endif

#ifdef HAS_ROUGHNESS_TEXTURE
layout(set=MATERIAL_SET_INDEX, binding = 4) uniform sampler2D metallic_tex;
#endif


#ifdef HAS_NORMAL_MAP
layout(set=MATERIAL_SET_INDEX, binding = 5) uniform sampler2D normal_map;
#endif

layout(location=0) in vec2 uv;
layout(location=0) out vec4 frag_color; // layout location ==> attachment index, refer to glsl specification 4.4.2 output layout qualifiers

void main(void)
{
    #ifdef HAS_BASE_COLOR_TEXTURE
    frag_color = texture(base_color_tex, uv);
    #else
    frag_color = pbr_mat.base_color;
    #endif
}