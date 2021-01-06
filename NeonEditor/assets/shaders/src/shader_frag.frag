#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_scalar_block_layout : enable

#include "material.glsl"

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec3 fragWorldPos;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) flat in int fragMatID;
layout(location = 5) in vec4 clipSpace;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0, scalar) readonly buffer MaterialBufferObject
{
    Material materials[];
};
layout(set = 0, binding = 1) uniform sampler2D textureSamplers[];

layout(push_constant, scalar) uniform PushConstant
{
    vec3 cameraPos;
    mat4 view;
    mat4 projection;

    vec4 clippingPlane;

    mat4 model;

    int pointLight;
    float lightIntensity;
    vec3 lightDirection;
    vec3 lightPosition;
}
pushConstant;

void main()
{
    Material mat = materials[fragMatID];

    vec3 lightDir = normalize(-pushConstant.lightDirection);
    float lightIntensity = pushConstant.lightIntensity;
    if (pushConstant.pointLight == 1)
    {
        lightDir = pushConstant.lightPosition - fragWorldPos;
        float d = length(lightDir);
        lightIntensity = pushConstant.lightIntensity / d;
    }

    vec2 normCoords = clipSpace.xy / clipSpace.w;
    normCoords = normCoords / 2 + 0.5;

    vec3 diffuse = computeDiffuse(mat, lightDir, fragNorm);
    if (mat.textureId >= 0)
    {
        vec3 diffuseTxt = texture(textureSamplers[mat.textureId], fragTexCoord).xyz;
        diffuse *= diffuseTxt;
    }

    vec3 viewDir = normalize(pushConstant.cameraPos - fragWorldPos);
    vec3 specular = computeSpecular(mat, viewDir, lightDir, fragNorm);

    float gamma = 1. / 2.2;
    outColor = pow(vec4(lightIntensity * (diffuse + specular), texture(textureSamplers[mat.textureId], fragTexCoord).w), vec4(gamma));
}