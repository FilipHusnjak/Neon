#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout (location = 0) flat in uint fragMaterialIndex;
layout (location = 1) in vec2 fragTexCoord;

layout (location = 0) out vec4 color;

layout (binding = 1) uniform sampler2D u_AlbedoTexture[];

void main()
{
    color = texture(u_AlbedoTexture[fragMaterialIndex], fragTexCoord);
}
