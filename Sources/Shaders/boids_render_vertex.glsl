#version 150 core

in vec2 position;
in vec2 velocity;

uniform float zoom;
uniform vec2 center;
uniform float boidLength;
uniform float boidWidth;

out Prim {
	out vec2 front;
	out vec2 right;
	out float boid_lenght;
	out float boid_width;
};

void main() {
	gl_Position = vec4(zoom * (position.x - center.x) / 500.0f, zoom * (position.y - center.y) / 500.0f, 0.0, 1.0);
	front = normalize(velocity);
	right = normalize(vec2(cross(vec3(front, 0.0f), vec3(0.0f, 0.0f, 1.0f))));
	boid_lenght = zoom * boidLength / 500.0f;
	boid_width = zoom * boidWidth / 500.0f;
}
