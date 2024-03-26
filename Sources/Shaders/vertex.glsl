#version 150 core

in vec2 position;
in float radius;

uniform float zoom;
uniform vec2 center;
uniform float maxRadius;
uniform vec4 bigColor;
uniform vec4 smallColor;

out vec4 color;

void main()
{
	gl_Position = vec4(zoom * (position.x - center.x) / 500.0f, zoom * (position.y - center.y) / 500.0f, 0.0, 1.0);
	color = mix(smallColor, bigColor, radius / maxRadius);
}
