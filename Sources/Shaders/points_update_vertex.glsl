#version 150 core

in vec2 position;
in vec2 velocity;

uniform float deltaTime;

out vec2 Position;
out vec2 Velocity;

void main() {
	Position = position + velocity * deltaTime;
	Velocity.x = (position.x > 500.0f) ? -abs(velocity.x) : (position.x < -500.0f) ? abs(velocity.x) : velocity.x;
	Velocity.y = (position.y > 500.0f) ? -abs(velocity.y) : (position.y < -500.0f) ? abs(velocity.y) : velocity.y;

	// Position = position + velocity * deltaTime;
	// Position.x = (Position.x > 500.0f) ? Position.x - 1000.0f : (Position.x < -500.0f) ? Position.x + 1000.0f : Position.x;
	// Position.y = (Position.y > 500.0f) ? Position.y - 1000.0f : (Position.y < -500.0f) ? Position.y + 1000.0f : Position.y;
	// Velocity = velocity;
}
