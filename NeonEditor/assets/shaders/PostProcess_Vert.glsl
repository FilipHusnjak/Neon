#version 430

layout (location = 0) in vec3 a_Position;

layout (location = 0) out vec2 v_TexCoord;

void main()
{
	vec4 position = vec4(a_Position.xy, 0.0, 1.0);
	v_TexCoord = (a_Position.xy + vec2(1.0)) / 2.0;
	gl_Position = position;
}