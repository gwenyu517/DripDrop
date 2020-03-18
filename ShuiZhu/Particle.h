
#ifndef PARTICLE_H_
#define PARTICLE_H_

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
	// stuffs
public:
	Particle();
	~Particle();
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
