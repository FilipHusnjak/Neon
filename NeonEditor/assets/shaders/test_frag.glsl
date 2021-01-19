#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout (location = 0) flat in uint v_MaterialIndex;
layout (location = 1) in vec2 v_TexCoord;

layout (location = 0) out vec4 o_Color;

layout (binding = 1) uniform sampler2D u_AlbedoTexture[];

void main()
{
    o_Color = texture(u_AlbedoTexture[v_MaterialIndex], v_TexCoord);
}
