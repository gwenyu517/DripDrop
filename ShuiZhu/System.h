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
	/* Regions
	 * 8 7 6
	 * 1 x 5
	 * 2 3 4
	 */
	//not really intuitive, but convenient; also, assuming gravity points down / no change in orientation
	enum class Region{L = 1, BL, B, BR, R, TR, T, TL};
	enum class Attrib{water, affinity};

	const float GRID_LENGTH;
	const int MAP_WIDTH;
	const int MAP_HEIGHT;
	const int MAP_SIZE;

	float minDropletMass;
	float maxDropletMass;

	int* id_map;	//hm...well if they overlap they were gonna merge anyway and they're
					// probably right next to each other anyway so just check if they're
					// neighboring should be enough?
	float* height_map;	// or double??
	float* affinity_map; // [0,1] generate on construction
	std::unordered_map<int, Particle*> particleList;

	void updateVelocity(double dt);
	void updatePosition(double dt);
	void leaveResidualDroplets(double dt);
	void updateHeightMap();

	void assignDropletShapes();
	void constructNewHeightMap();
	void smoothHeightMap();
	void erodeHeightMap();
	bool isBoundaryCell(int i, int j);
	void mergeDroplets();
	int index(glm::vec2 pos);
	int index(int x, int y);
	glm::vec2 position(int index);
	glm::vec2 position(int x, int y);
	glm::ivec2 gridPosition(int index);

	Region determineDirectionOfMovement(Particle* p);
	float sumOf(glm::vec2 pos, Region region, Attrib attrib);


	void check();

public:
	//System();
	System(float width, float height, float gridLength);
	//System(int width, int height, int initialNumOfParticles);
	~System();

	void update(double dt);
	float* getHeightMap();
};


#endif /* SYSTEM_H_ */
