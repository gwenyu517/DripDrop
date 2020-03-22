#include "Particle.h"

Particle::Particle(int x, int y) {
	position.x = x;
	position.y = y;
	timeSinceLastResidual = 0.0;

}

Particle::~Particle() {

}

void Particle::resetTimeSinceLastResidual() {
	timeSinceLastResidual = 0.0;
}

bool Particle::leaveResidual() {
	return false;
}

bool Particle::isStatic() {
	return false;
}

bool Particle::isResidual() {
	return timeSinceLastResidual == 0.0;
}

glm::ivec2 Particle::getPosition() {
	return position;
}
glm::vec2 Particle::getVelocity() {
	return velocity;
}
float Particle::getMass() {
	return mass;
}
float Particle::getMass_static() {
	return mass_static;
}

void Particle::setPosition(glm::ivec2 p) {
	position = p;
}
void Particle::setVelocity(glm::vec2 v) {
	velocity = v;
}
void Particle::setMass(float m) {
	mass = m;
}
