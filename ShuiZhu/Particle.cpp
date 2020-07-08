#include "Particle.h"
#include <algorithm>
#include <iostream>
//#include <glm/gtc/constants.hpp>


/*Particle::Particle() {

}*/

//const float Particle::mass_static =  0.05f;

//const float PI = 3.14159265359f;

int Particle::nextID = 0;
float Particle::mass_static =  1.0f;
float Particle::density = 1.0f;
//	double maxResidualTime = 0.4;

Particle::Particle() {

};

Particle::Particle(glm::vec2 p, float m) {
	id = nextID++;
	parentID = -1;
	position = p;
	velocity = glm::vec2(0,0);
	timeSinceLastResidual = 0.0;
	mass = m;
	//mass_static =
	radius = 0.0f;
}

Particle::~Particle() {
	std::cout << "dead pid " << id << std::endl;
}

void Particle::resetTimeSinceLastResidual() {
	timeSinceLastResidual = 0.0;
}

void Particle::addResidualTime(double dt) {
	timeSinceLastResidual += dt;
}

bool Particle::leaveResidual(double dt, float chance) {
	// probability of leaving drop at current time step =
	// min(1,
	//   beta * (dt / t_max_residualTime) *
	//			min(1, timeSinceLastResidual / t_max_residualTime))
	// input needed is t_max_residualTime, dt, and timeSinceLastResidual
	// they set beta = 3, and dt <= t_max / 3, with t_max = 0.4


	double a = beta * (dt/maxResidualTime) * std::min(1.0, timeSinceLastResidual/maxResidualTime);
	//std::cout << "a is " << a << std::endl;
	double probability = std::min(1.0, a);
	std::cout << "dt = " << dt << ", a = " << a << std::endl;

	return (chance <= probability);
}

bool Particle::isStatic() {
	return mass <= mass_static;
}

bool Particle::isResidualOf(int particleID) {
	return (parentID == particleID) && (mass <= mass_static);
//	return (timeSinceLastResidual == 0.0) && (mass <= mass_static);
}

//bool Particle::isChildOf(Particle &particle) {
//	return parentID == particle.getID();
//}

/**** GETTERS ****/

int Particle::getNextID() {
	return nextID;
}
int Particle::getID() {
	return id;
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

//int Particle::getParentID() {
//	return parentID;
//}

/**** SETTERS ****/

void Particle::mergeID(int id) {
	this->id = id;
}

void Particle::setParent(int id) {
	parentID = id;
}

void Particle::clearParent() {
	parentID = -1;
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
