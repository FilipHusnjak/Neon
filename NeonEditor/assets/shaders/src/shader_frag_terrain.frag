#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_scalar_block_layout : enable

#include "material.glsl"

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec3 fragWorldPos;
layout(location = 3) in vec2 fragMapTexCoord;
layout(location = 4) in vec2 fragTileTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0, scalar) readonly buffer MaterialBufferObject
{
    Material material;
};

layout(set = 0, binding = 1) uniform sampler2D blendMap;
layout(set = 0, binding = 2) uniform sampler2D backgroundTexture;
layout(set = 0, binding = 2) uniform sampler2D rTexture;
layout(set = 0, binding = 3) uniform sampler2D gTexture;
layout(set = 0, binding = 4) uniform sampler2D bTexture;

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
    vec3 blendColor = texture(blendMap, fragMapTexCoord).rgb;
    float background = 1.0 - (blendColor.r + blendColor.g + blendColor.b);
    vec4 finalTextureColor = background * texture(backgroundTexture, fragTileTexCoord) + blendColor.r * texture(rTexture, fragTileTexCoord) +
    blendColor.g * texture(gTexture, fragTileTexCoord) + blendColor.b * texture(bTexture, fragTileTexCoord);

    vec3 lightDir = normalize(-pushConstant.lightDirection);
    float lightIntensity = pushConstant.lightIntensity;
    if (pushConstant.pointLight == 1)
    {
        lightDir = pushConstant.lightPosition - fragWorldPos;
        float d = length(lightDir);
        lightIntensity = pushConstant.lightIntensity / d;
    }

    vec3 diffuse = computeDiffuse(material, lightDir, fragNorm) * finalTextureColor.rgb;

    vec3 viewDir = normalize(pushConstant.cameraPos - fragWorldPos);
    vec3 specular = computeSpecular(material, viewDir, lightDir, fragNorm);

    float gamma = 1. / 2.2;
    outColor = pow(vec4(lightIntensity * (diffuse + specular), finalTextureColor.a), vec4(gamma));
}