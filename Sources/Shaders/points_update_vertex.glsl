#version 150 core

in vec2 position;
in vec2 velocity;

uniform float deltaTime;

out vec2 Position;
out vec2 Velocity;

void main() {
	// bounce
	// Position = position + velocity * deltaTime;
	// Velocity.x = (position.x > 500.0f) ? -abs(velocity.x) : (position.x < -500.0f) ? abs(velocity.x) : velocity.x;
	// Velocity.y = (position.y > 500.0f) ? -abs(velocity.y) : (position.y < -500.0f) ? abs(velocity.y) : velocity.y;

	// wrap around
	// Position = position + velocity * deltaTime;
	// Position.x = (Position.x > 500.0f) ? Position.x - 1000.0f : (Position.x < -500.0f) ? Position.x + 1000.0f : Position.x;
	// Position.y = (Position.y > 500.0f) ? Position.y - 1000.0f : (Position.y < -500.0f) ? Position.y + 1000.0f : Position.y;
	// Velocity = velocity;

	// keep within bounds
	Position = position + velocity * deltaTime;
	Velocity.x = (Position.x > 500.0f) ? velocity.x - 1.0f : (Position.x < -500.0f) ? velocity.x + 1.0f : velocity.x;
	Velocity.y = (Position.y > 500.0f) ? velocity.y - 1.0f : (Position.y < -500.0f) ? velocity.y + 1.0f : velocity.y;
}
