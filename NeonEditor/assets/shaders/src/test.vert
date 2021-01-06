#version 450

layout(location = 0) in vec3 pos;

layout(set = 0, binding = 0) uniform CameraUBO
{
    mat4 model;
    mat4 view;
    mat4 projection;
};

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0);
}
