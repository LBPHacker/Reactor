#version 450 core

layout (location = 0) in vec2 pos;
out vec2 uv;

void main()
{
	gl_Position = vec4(pos, 0, 1);
	uv = pos;
}
