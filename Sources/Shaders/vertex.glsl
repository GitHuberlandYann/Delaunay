#version 150 core

in vec2 position;
in float radius;

uniform float maxRadius;
uniform vec4 bigColor;
uniform vec4 smallColor;

out vec4 color;

void main()
{
	gl_Position = vec4(position.x / 500.0f, position.y / 500.0f, 0.0, 1.0);
	color = mix(smallColor, bigColor, radius / maxRadius);
}
