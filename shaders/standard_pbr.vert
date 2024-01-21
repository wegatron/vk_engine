#version 450
#extension GL_EXT_scalar_block_layout : require

// vertex data binding = 0
layout(location=0) in vec4 vpos;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;

layout(location=0) out vec2 out_uv;
layout(location=1) out vec3 out_normal;
layout(location=2) out vec3 out_pos;

struct LightT
{
  int light_type;
  float angle;
  float blend;
  float falloff;//16

  vec3 position;
  vec3 direction;
  vec3 color; // 64
  float intensity; // 64
};

layout(std430, set=GLOBAL_SET_INDEX, binding = 0) uniform GlobalUniform
{
    // camera
    mat4 view; // 64
    mat4 proj; // 128

    // lights
    LightT lights[MAX_LIGHTS_COUNT]; // 128 + 64 * MAX_LIGHTS_COUNT
    uint light_count;  // 128 + 64 * MAX_LIGHTS_COUNT + 16
} global_uniform;

layout(set=PER_OBJECT_SET_INDEX, binding = 0) uniform MeshUniform
{
    mat4 model;
} mesh_uniform;

void main(void)
{
    out_uv = uv;
    out_normal = (mesh_uniform.model * vec4(normal, 1.0f)).xyz;
    vec4 gpos = mesh_uniform.model * vpos;
    out_pos = gpos.xyz;
    gl_Position = global_uniform.proj * global_uniform.view * gpos;
}