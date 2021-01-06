#version 450

layout(location = 0) out vec4 outColor;

layout(std140, binding = 1) uniform Color
{
    float c;
} colors[2];

void main()
{
    outColor = vec4(colors[0].c, colors[1].c, 0.f, 1.f);
}
