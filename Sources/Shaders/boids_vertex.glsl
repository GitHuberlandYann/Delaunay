#version 150 core

in vec2 position;
in vec2 velocity;

uniform float deltaTime;
uniform samplerBuffer boids;
uniform int nbBoids;
uniform float visualRange;
uniform float centeringFactor; // 0.005 used in coherence
uniform float minDistance; // 20 used in separation, dist to stay away from other boids
uniform float avoidFactor; // 0.05 used in separation
uniform float matchingFactor; // 0.05 used in alignment

out vec2 Position;
out vec2 Velocity;

// Find the center of mass of the other boids and adjust velocity slightly to
// point towards the center of mass.
void coherence( void ) {
	float centerX = 0; // TODO use vec2
	float centerY = 0;
	int numNeighbors = 0;

	for (int i = 0; i < nbBoids; ++i) {
		vec4 boid = texelFetch(boids, i);
		if (distance(position, boid.xy) < visualRange) {
			centerX += boid.x;
			centerY += boid.y;
			++numNeighbors;
		}
	}

	if (numNeighbors != 0) {
		centerX /= numNeighbors;
		centerY /= numNeighbors;

		Velocity.x += (centerX - position.x) * centeringFactor;
		Velocity.y += (centerY - position.y) * centeringFactor;
	}
}

// Move away from other boids that are too close to avoid colliding
void separation( void ) {
	float moveX = 0;
	float moveY = 0;
	for (int i = 0; i < nbBoids; ++i) {
		if (i != gl_VertexID) {
			vec4 boid = texelFetch(boids, i);
			if (distance(position, boid.xy) < minDistance) {
				moveX += position.x - boid.x;
				moveY += position.y - boid.y;
			}
		}
	}

	Velocity.x += moveX * avoidFactor;
	Velocity.y += moveY * avoidFactor;
}

// Find the average velocity (speed and direction) of the other boids and
// adjust velocity slightly to match.
void alignment( void ) {
	float avgDX = 0;
	float avgDY = 0;
	int numNeighbors = 0;

	for (int i = 0; i < nbBoids; ++i) {
		vec4 boid = texelFetch(boids, i);
		if (distance(position, boid.xy) < visualRange) {
			avgDX += boid.z;
			avgDY += boid.w;
			++numNeighbors;
		}
	}

	if (numNeighbors != 0) {
		avgDX /= numNeighbors;
		avgDY /= numNeighbors;

		Velocity.x += (avgDX - Velocity.x) * matchingFactor;
		Velocity.y += (avgDY - Velocity.y) * matchingFactor;
	}
}

void limitSpeed( void ) {
	const float speedLimit = 15 * 15;

	float speed = Velocity.x * Velocity.x + Velocity.y * Velocity.y;
	if (speed > speedLimit) {
		Velocity.x = (Velocity.x / speed) * speedLimit;
		Velocity.y = (Velocity.y / speed) * speedLimit;
	}
}

void keepWithinBounds( void ) {
	const float marginMin = -300;
	const float marginMax = 300;
	const float turnFactor = 1;

	if (position.x < marginMin) {
		Velocity.x += turnFactor;
	}
	if (position.x > marginMax) {
		Velocity.x -= turnFactor;
	}
	if (position.y < marginMin) {
		Velocity.y += turnFactor;
	}
	if (position.y > marginMax) {
		Velocity.y -= turnFactor;
	}
}

void main() {
	Velocity = velocity;

	// Update the velocities according to each rule
    coherence();
    separation();
    alignment();
    limitSpeed();
    keepWithinBounds();

    // Update the position based on the current velocity
    Position = position + Velocity;// * deltaTime;
}
