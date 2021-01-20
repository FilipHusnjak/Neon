#version 450

layout(location = 0) in vec3 v_WorldPos;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec4 v_ClipSpace;
layout(location = 3) in vec2 v_TexCoordinates;

layout(location = 0) out vec4 o_Color;

layout(binding = 1) uniform sampler2D u_RefractionTexture;
layout(binding = 2) uniform sampler2D u_ReflectionTexture;
layout(binding = 3) uniform sampler2D u_DudvMap;
layout(binding = 4) uniform sampler2D u_NormalMap;
layout(binding = 5) uniform sampler2D u_DepthMap;

layout(push_constant) uniform PushConstant
{
    vec4 u_CameraPosition;

    vec4 u_LightDirection;
    vec4 u_LightPosition;

    vec4 u_ClippingPlane;

    int u_IsPointLight;
    float u_LightIntensity;

    float u_MoveFactor;

    float u_Near;
    float u_Far;
};

float waveStrength = 0.15;

vec3 computeSpecular(vec3 viewDir, vec3 lightDir, vec3 normal)
{
    const float kShininess = 4.0;
    if (dot(lightDir, normal) < 0) return vec3(0);
    vec3 r = normalize(2 * dot(lightDir, normal) * normal - lightDir);
    float specular = pow(max(dot(r, viewDir), 0.0), kShininess);
    return vec3(0.9 * specular);
}

void main()
{
    vec3 lightDir = normalize(-u_LightDirection.xyz);
    float lightIntensity = u_LightIntensity;
    if (u_IsPointLight > 0)
    {
        lightDir = u_LightPosition.xyz - v_WorldPos;
        lightIntensity /= length(lightDir);
        lightDir = normalize(lightDir);
    }

    vec3 viewDir = normalize(u_CameraPosition.xyz - v_WorldPos);

    vec2 textureCoords = v_ClipSpace.xy / v_ClipSpace.w;
    textureCoords = textureCoords / 2 + 0.5;

    vec2 refractTextureCoords = vec2(textureCoords.x, textureCoords.y);
    vec2 reflectTextureCoords = vec2(textureCoords.x, -textureCoords.y);

    float floorDist = 2.0 * u_Near * u_Far / (u_Far + u_Near - texture(u_DepthMap, refractTextureCoords).r * (u_Far - u_Near));

    float waterDist = 2.0 * u_Near * u_Far / (u_Far + u_Near - gl_FragCoord.z * (u_Far - u_Near));
    
    float waterDepth = floorDist - waterDist;

    vec2 distortedTexCoords = texture(u_DudvMap, vec2(v_TexCoordinates.x + u_MoveFactor, v_TexCoordinates.y)).rg * 0.1;
    distortedTexCoords = v_TexCoordinates + vec2(distortedTexCoords.x, distortedTexCoords.y + u_MoveFactor);
    vec2 distortion = (texture(u_DudvMap, distortedTexCoords).rg * 2.0 - 1.0) * waveStrength / waterDist;

    refractTextureCoords += distortion;
    refractTextureCoords = clamp(refractTextureCoords, 0.001, 0.999);

    reflectTextureCoords += distortion;
    reflectTextureCoords.x = clamp(reflectTextureCoords.x, 0.001, 0.999);
    reflectTextureCoords.y = clamp(reflectTextureCoords.y, -0.999, -0.001);

    vec4 normalMapColor = texture(u_NormalMap, distortedTexCoords);
    vec3 normal = normalize(vec3(normalMapColor.r * 2.0 - 1.0, normalMapColor.b * 3, normalMapColor.g * 2.0 - 1.0));

    float refractiveFactor = pow(abs(dot(viewDir, normal)), 0.5);
    vec4 textureValue = mix(texture(u_ReflectionTexture, reflectTextureCoords), texture(u_RefractionTexture, refractTextureCoords), refractiveFactor);

    vec3 specular = computeSpecular(viewDir, lightDir, normal) * clamp(waterDepth / 5.0, 0.0, 1.0);

    o_Color = (mix(textureValue, vec4(0.0, 0.3, 0.5, 1.0), 0.2) + lightIntensity * vec4(specular, 0.0));
    o_Color.a = clamp(waterDepth / 5.0, 0.0, 1.0);
}