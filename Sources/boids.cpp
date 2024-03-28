#include "boids.hpp"
#include "math.h"

static float distance( Vertex a, Vertex b )
{
	return std::sqrt((a.getX() - b.getX()) * (a.getX() - b.getX()) +
      (a.getY() - b.getY()) * (a.getY() - b.getY()));
}

// Find the center of mass of the other boids and adjust velocity slightly to
// point towards the center of mass.
void coherence( t_updateShaderInput &boid, std::vector<t_updateShaderInput> &boids, t_boidSettings boidSettings ) {
	float centerX = 0; // TODO use vec2
	float centerY = 0;
	int numNeighbors = 0;

	for (auto &other : boids) {
		if (distance(boid.v, other.v) < boidSettings.visualRange) {
			centerX += other.v.getX();
			centerY += other.v.getY();
			++numNeighbors;
		}
	}

	if (numNeighbors != 0) {
		centerX /= numNeighbors;
		centerY /= numNeighbors;

		boid.speed.addV(Vertex((centerX - boid.v.getX()) * boidSettings.centeringFactor,
			(centerY - boid.v.getY()) * boidSettings.centeringFactor));
	}
}

// Move away from other boids that are too close to avoid colliding
void separation( t_updateShaderInput &boid, std::vector<t_updateShaderInput> &boids, t_boidSettings boidSettings ) {
	float moveX = 0;
	float moveY = 0;

	for (auto &other : boids) {
		if (&boid != &other) {
			if (distance(boid.v, other.v) < boidSettings.minDistance) {
				moveX += boid.v.getX() - other.v.getX();
				moveY += boid.v.getY() - other.v.getY();
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
				avgDX += other.speed.getX();
				avgDY += other.speed.getY();
				++numNeighbors;
			}
		}
	}

	if (numNeighbors != 0) {
		avgDX /= numNeighbors;
		avgDY /= numNeighbors;

		boid.speed.addV(Vertex((avgDX - boid.speed.getX()) * boidSettings.matchingFactor,
			(avgDY - boid.speed.getY()) * boidSettings.matchingFactor));
	}
}

void limitSpeed( t_updateShaderInput &boid ) {
	const float speedLimit = 100 * 100;

	float speed = boid.speed.getX() * boid.speed.getX() + boid.speed.getY() * boid.speed.getY();
	if (speed > speedLimit) {
		boid.speed = Vertex((boid.speed.getX() / speed) * speedLimit, (boid.speed.getY() / speed) * speedLimit);
	}
}

void keepWithinBounds( t_updateShaderInput &boid ) {
	const float marginMin = -450;
	const float marginMax = 450;
	const float turnFactor = 1;

	if (boid.v.getX() < marginMin) {
		boid.speed.addX(turnFactor);
	} else if (boid.v.getX() > marginMax) {
		boid.speed.addX(-turnFactor);
	}
	if (boid.v.getY() < marginMin) {
		boid.speed.addY(turnFactor);
	} else if (boid.v.getY() > marginMax) {
		boid.speed.addY(-turnFactor);
	}
}

void update_boids( std::vector<t_updateShaderInput> &boids, t_boidSettings boidSettings, float deltaTime )
{
	(void)boidSettings;
	for (auto &b : boids) {
		// Update the velocities according to each rule
		coherence(b, boids, boidSettings);
		separation(b, boids, boidSettings);
		alignment(b, boids, boidSettings);
		limitSpeed(b);
		keepWithinBounds(b);

		// Update the position based on the current velocity
		b.v.addV(b.speed.scale(deltaTime));
	}
}
