#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_scalar_block_layout : enable

layout(location = 0) in vec3 pos;

layout(location = 0) out vec3 fragWorldPos;

layout(push_constant, scalar) uniform PushConstant
{
    vec3 cameraPos;
    mat4 view;
    mat4 projection;

    vec4 clippingPlane;

    mat4 model;
}
pushConstant;

void main()
{
    vec4 worldPos = pushConstant.model * vec4(pos, 1.0);
    fragWorldPos = (worldPos).xyz;

    gl_ClipDistance[0] = dot(worldPos, pushConstant.clippingPlane);

    gl_Position = pushConstant.projection * pushConstant.view * worldPos;
}