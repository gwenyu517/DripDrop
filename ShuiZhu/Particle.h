
#ifndef PARTICLE_H_
#define PARTICLE_H_

#include <glm/glm.hpp>
#include <vector>
#include <unordered_set>

class Particle {
private:
	glm::vec2 position;
	glm::vec2 velocity;
	float mass;
	float radius;
	double timeSinceLastResidual;

	static float mass_static;	// grams
	double maxResidualTime = 0.4;
	int beta = 3;
	static float density;


	std::vector<glm::vec2> q;
	std::unordered_set<int> occupiedCells;

public:
	//Particle();
	Particle(glm::vec2 p, float m);
	~Particle();

	void resetTimeSinceLastResidual();
	void addResidualTime(double dt);
	bool leaveResidual(double dt, float chance);
	bool isStatic();
	bool isResidual();

	glm::vec2 getPosition();
	glm::vec2 getVelocity();
	float getMass();
	static float getMass_static();
	double getMaxResidualTime();
	double getTimeSinceLastResidual();
	float getRadius();
	static float getDensity();
	std::vector<glm::vec2> getHemispherePositions();
	std::unordered_set<int> getListOfOccupiedCells();

	void setPosition(glm::vec2 p);
	void setVelocity(glm::vec2 v);
	void setMass(float m);
	static void setMass_static(float m);
	void setRadius(float r);
	void addHemispherePosition(glm::vec2 p);
	void clearHemispherePositions();
	void addOccupiedCells(int index);
	void addOccupiedCells(std::unordered_set<int> list);
	void removeOccupiedCells(int index);


};

/*
 * 	1. apply external force (a)
 * 	2. choose direction of movement (a)
 * 	3. leave residual droplets
 * 	4. assign shape	(b)
 * 	5. compute height map (b)
 * 	6. smooth / erosion height map
 * 	7. merging droplets
 */



/* class system
 * 		array of particle objects
 * 		array of heights (height map)
 * 		array of IDs (ID map)
 * 		array of affinities (affinity map)
 *
 * 		update velocity() (functions 1+2)	--> modify particles, use affinity map
 * 		update residual droplets()			--> modify particles / array of particles, use height map
 *
 * 		update height map()
 *			create new height map (functions 4+5)	--> use array of particles, ID map, height map
 *				essentially updating the old height map/ID map (do not start from a clean slate)
 *			smooth()	(function 6a)			--> only new height map
 *			erode()		(function 6b)			--> only new height map
 *			merge droplets()					--> modify array of particles
 */

#endif /* PARTICLE_H_ */
