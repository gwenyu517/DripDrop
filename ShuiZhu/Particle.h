
#ifndef PARTICLE_H_
#define PARTICLE_H_

#include <glm/glm.hpp>

/* particles have
 * 		position
 * 		mass
 * 		velocity
 * 		ID
 * 		time elapsed since previous residual water drop created
 *
 *
 * 		reset time elapsed()
 * 		bool leaveResidual()
 *		bool isStatic()
 *		bool isResidual()
 */

class Particle {
private:
	glm::vec2 position;
	glm::vec2 velocity;
	float mass;
	double timeSinceLastResidual;
	float mass_static =  0.000050f;
	double maxResidualTime = 0.4;

public:
	//Particle();
	Particle(glm::vec2 p, float m);
	~Particle();

	void resetTimeSinceLastResidual();
	void addResidualTime(double dt);
	bool leaveResidual(int beta, double dt);
	bool isStatic();
	bool isResidual();

	glm::vec2 getPosition();
	glm::vec2 getVelocity();
	float getMass();
	float getMass_static();
	double getMaxResidualTime();
	double getTimeSinceLastResidual();

	void setPosition(glm::vec2 p);
	void setVelocity(glm::vec2 v);
	void setMass(float m);

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
