#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <unordered_map>
#include "particle.h"

/* class system
 * 		array of particle objects --> unordered map of particles, with key=ID
 * 		array of heights (height map)
 * 		array of IDs (ID map)
 * 		array of affinities (affinity map)
 *
 * 		update velocity() (functions 1+2)	--> modify particles, use affinity map
 * 		update residual droplets()			--> modify particles / array of particles, use height map
 *
 * 		update height map()
 *			construct new height map (functions 4+5)	--> use array of particles, ID map, height map
 *				essentially updating the old height map/ID map (do not start from a clean slate)
 *			smooth()	(function 6a)			--> only new height map
 *			erode()		(function 6b)			--> only new height map
 *			merge droplets()					--> modify array of particles
 */

class System {
private:
	enum class Region{BL, B, BR};	// this assumes that gravity points "down" on the grid / no change in surface orientation
	enum class Attrib{water, affinity};

	const int WIDTH;
	const int HEIGHT;
	const int SIZE;

	int* id_map;
	float* height_map;	// or double??
	float* affinity_map; // [0,1] generate on construction
	std::unordered_map<int, Particle> particles;

	void updateVelocity(double dt);
	void leaveResidualDroplets();
	void updateHeightMap();
	void constructNewHeightMap();
	void smoothHeightMap();
	void erodeHeightMap();
	void mergeDroplets();
	int p(glm::ivec2 pos);
	int p(int x, int y);

	Region determineDirectionOfMovement(Particle p);
	float sumOf(glm::ivec2 pos, Region region, Attrib attrib);

public:
	//System();
	System(int width, int height);
	//System(int width, int height, int initialNumOfParticles);
	~System();

	void update(double dt);
};


#endif /* SYSTEM_H_ */
