#include "boids.hpp"

/*
// Find the center of mass of the other boids and adjust velocity slightly to
// point towards the center of mass.
void coherence( t_updateShaderInput &boid, std::vector<t_updateShaderInput> &boids, t_boidSettings boidSettings ) {
	float centerX = 0; // TODO use vec2
	float centerY = 0;
	int numNeighbors = 0;

	for (auto &other : boids) {
		if (distance(boid.v, other.v) < boidSettings.visualRange) {
			centerX += other.v.x;
			centerY += other.v.y;
			++numNeighbors;
		}
	}

	if (numNeighbors != 0) {
		centerX /= numNeighbors;
		centerY /= numNeighbors;

		boid.speed.addV(Vertex((centerX - boid.v.x) * boidSettings.centeringFactor,
			(centerY - boid.v.y) * boidSettings.centeringFactor));
	}
}

// Move away from other boids that are too close to avoid colliding
void separation( t_updateShaderInput &boid, std::vector<t_updateShaderInput> &boids, t_boidSettings boidSettings ) {
	float moveX = 0;
	float moveY = 0;

	for (auto &other : boids) {
		if (&boid != &other) {
			if (distance(boid.v, other.v) < boidSettings.minDistance) {
				moveX += boid.v.x - other.v.x;
				moveY += boid.v.y - other.v.y;
			}
		}
	}

	boid.speed.addV(Vertex(moveX * boidSettings.avoidFactor, moveY * boidSettings.avoidFactor));
}

// Find the average velocity (speed and direction) of the other boids and
// adjust velocity slightly to match.
void alignment( t_updateShaderInput &boid, std::vector<t_updateShaderInput> &boids, t_boidSettings boidSettings ) {
	float avgDX = 0;
	float avgDY = 0;
	int numNeighbors = 0;

	for (auto &other : boids) {
		if (&boid != &other) {
			if (distance(boid.v, other.v) < boidSettings.visualRange) {
				avgDX += other.speed.x;
				avgDY += other.speed.y;
				++numNeighbors;
			}
		}
	}

	if (numNeighbors != 0) {
		avgDX /= numNeighbors;
		avgDY /= numNeighbors;

		boid.speed.addV(Vertex((avgDX - boid.speed.x) * boidSettings.matchingFactor,
			(avgDY - boid.speed.y) * boidSettings.matchingFactor));
	}
}*/

void applyRules( t_updateShaderInput &boid, std::vector<t_updateShaderInput> &boids, t_boidSettings boidSettings )
{
	Vertex center(0.0f, 0.0f);
	int numNeighborsCoherence = 0;
	Vertex move(0.0f, 0.0f);
	Vertex avg(0.0f, 0.0f);
	int numNeighborsAlignment = 0;

	for (auto &other : boids) {
		float dist = boid.v.distance(other.v);
		if (dist < boidSettings.visualRange) {
			// coherence
			center += other.v;
			++numNeighborsCoherence;
		}

		if (&boid != &other) {
			// separation
			if (dist < boidSettings.minDistance) {
				move += boid.v - other.v;
			}

			// alignment
			if (dist < boidSettings.visualRange) {
				avg += other.speed;
				++numNeighborsAlignment;
			}
		}
	}

	// coherence
	if (numNeighborsCoherence != 0) {
		center /= numNeighborsCoherence;
		boid.speed += (center - boid.v) * boidSettings.centeringFactor;
	}

	// separation
	boid.speed += move * boidSettings.avoidFactor;

	// alignment
	if (numNeighborsAlignment != 0) {
		avg /= numNeighborsAlignment;

		boid.speed += (avg - boid.speed) * boidSettings.matchingFactor;
	}
}

void limitSpeed( t_updateShaderInput &boid, float speedLimit ) {
	speedLimit *= speedLimit;

	float speed = boid.speed.x * boid.speed.x + boid.speed.y * boid.speed.y;
	if (speed > speedLimit) {
		boid.speed *= speedLimit / speed;
	}
}

void keepWithinBounds( t_updateShaderInput &boid ) {
	const float marginMin = -450;
	const float marginMax = 450;
	const float turnFactor = 1;

	if (boid.v.x < marginMin) {
		boid.speed.x += turnFactor;
	} else if (boid.v.x > marginMax) {
		boid.speed.x -= turnFactor;
	}
	if (boid.v.y < marginMin) {
		boid.speed.y += turnFactor;
	} else if (boid.v.y > marginMax) {
		boid.speed.y -= turnFactor;
	}
}

void update_boids( std::vector<t_updateShaderInput> &boids, t_boidSettings boidSettings, float deltaTime )
{
	(void)boidSettings;
	for (auto &b : boids) {
		// Update the velocities according to each rule
		// coherence(b, boids, boidSettings);
		// separation(b, boids, boidSettings);
		// alignment(b, boids, boidSettings);
		applyRules(b, boids, boidSettings);
		limitSpeed(b, boidSettings.speedLimit);
		keepWithinBounds(b);

		// Update the position based on the current velocity
		b.v += b.speed * deltaTime;
	}
}
