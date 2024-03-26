#version 150 core

in vec2 position;
in vec2 velocity;

uniform float zoom;
uniform vec2 center;

void main() {
	gl_PointSize = 3.0f;
	gl_Position = vec4(zoom * (position.x - center.x) / 500.0f, zoom * (position.y - center.y) / 500.0f, 0.0, 1.0);
}
