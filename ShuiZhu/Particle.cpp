#include "Particle.h"
#include <algorithm>

/*Particle::Particle() {

}*/
Particle::Particle(glm::vec2 p, float m) {
	position = p;
	velocity = glm::vec2(0,0);
	timeSinceLastResidual = 0.0;
	mass = m;

}

Particle::~Particle() {

}

void Particle::resetTimeSinceLastResidual() {
	timeSinceLastResidual = 0.0;
}

void Particle::addResidualTime(double dt) {
	timeSinceLastResidual += dt;
}

bool Particle::leaveResidual(int beta, double dt) {
	// probability of leaving drop at current time step =
	// min(1,
	//   beta * (dt / t_max_residualTime) *
	//			min(1, timeSinceLastResidual / t_max_residualTime))
	// input needed is t_max_residualTime, dt, and timeSinceLastResidual
	// they set beta = 3, and dt <= t_max / 3, with t_max = 0.4

	double a = beta * (dt/maxResidualTime) * std::min(1.0, timeSinceLastResidual/maxResidualTime);
	return std::min(1.0, a);
}

bool Particle::isStatic() {
	return mass <= mass_static;
}

bool Particle::isResidual() {
	return timeSinceLastResidual == 0.0;
}

glm::vec2 Particle::getPosition() {
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

double Particle::getMaxResidualTime() {
	return maxResidualTime;
}
double Particle::getTimeSinceLastResidual() {
	return timeSinceLastResidual;
}

void Particle::setPosition(glm::vec2 p) {
	position = p;
}
void Particle::setVelocity(glm::vec2 v) {
	velocity = v;
}
void Particle::setMass(float m) {
	mass = m;
}
