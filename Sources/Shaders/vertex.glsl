#version 150 core

in vec2 position;
in float radius;

uniform float maxRadius;

out float greyScale;

void main()
{
	gl_Position = vec4(position.x / 500.0f, position.y / 500.0f, 0.0, 1.0);
	greyScale = mix(0.2f, 0.8f, radius / maxRadius);
}
