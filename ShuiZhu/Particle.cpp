#include "Particle.h"
#include <algorithm>
#include <iostream>
//#include <glm/gtc/constants.hpp>


/*Particle::Particle() {

}*/

//const float Particle::mass_static =  0.05f;

//const float PI = 3.14159265359f;

float Particle::mass_static =  1.0f;
float Particle::density = 1.0f;

Particle::Particle(glm::vec2 p, float m) {
	position = p;
	velocity = glm::vec2(0,0);
	timeSinceLastResidual = 0.0;
	mass = m;
	//mass_static =
	radius = 0.0f;
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
	//std::cout << "a is " << a << std::endl;
	double probability = std::min(1.0, a);

// you need to use the probability dumb dumb lol

	return false;
}

bool Particle::isStatic() {
	return mass <= mass_static;
}

bool Particle::isResidual() {
	return (timeSinceLastResidual == 0.0) && (mass <= mass_static);
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

float Particle::getRadius() {
	return radius;
}

float Particle::getDensity() {
	return density;
}

std::vector<glm::vec2> Particle::getHemispherePositions() {
	return q;
}

std::unordered_set<int> Particle::getListOfOccupiedCells() {
	return occupiedCells;
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

void Particle::setMass_static(float m) {
	mass_static = m;
}

void Particle::setRadius(float r) {
	radius = r;
}

void Particle::addHemispherePosition(glm::vec2 p) {
	q.push_back(p);
}

void Particle::clearHemispherePositions() {
	q.clear();
}

void Particle::addOccupiedCells(int index) {
	occupiedCells.insert(index);
}

void Particle::addOccupiedCells(std::unordered_set<int> list) {
	occupiedCells.insert(list.begin(), list.end());
}

void Particle::removeOccupiedCells(int index) {
	occupiedCells.erase(index);
}
