#version 150 core

in vec2 position;
in float radius;

out float greyScale;

void main()
{
	gl_Position = vec4(position, 0.0, 1.0);
	greyScale = min(1.0f, 0.2f + radius * 0.5f);
}
