#version 150 core

in vec2 position;
in vec2 velocity;

void main() {
	gl_PointSize = 3.0f;
	gl_Position = vec4(position.x / 500.0f, position.y / 500.0f, 0.0, 1.0);
}
