
#ifndef SYSTEM_NP_H_
#define SYSTEM_NP_H_

#include <unordered_map>
#include <vector>
#include "particle.h"


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

	std::vector< std::vector<int> > id_map;
	//hm...well if they overlap they were gonna merge anyway and they're
					// probably right next to each other anyway so just check if they're
					// neighboring should be enough?
	float* height_map;	// or double??
	float* affinity_map; // [0,1] generate on construction
	std::unordered_map<int, Particle> particleList;

	void generateParticles();

	void updateVelocity(double dt);
	void updatePosition(double dt);
	void leaveResidualDroplets(double dt);
	void updateHeightMap();

	void assignDropletShapes();

	void constructNewHeightMap();
	void placeHemisphere(Particle &particle, glm::vec2 hemispherePosition);

	bool isOutOfBounds(int i, int j);

	void smoothHeightMap();

	void erodeHeightMap();
	bool isResidualDropletCell(int i, int j);
	bool isBoundaryCell(int i, int j);

	void deleteOutOfBoundDroplets();

	void mergeDroplets();
	int neighboringDroplet(Particle &particle);
	void mergeParticles(std::unordered_map<int, Particle> &list, int particleID, int neighborID);

	int index(glm::vec2 pos);
	int index(int x, int y);
	glm::vec2 position(int index);
	glm::vec2 position(int x, int y);
	glm::ivec2 gridPosition(int index);
	glm::ivec2 gridPosition(glm::vec2 pos);

	Region determineDirectionOfMovement(Particle p);
	float sumOf(glm::vec2 pos, Region region, Attrib attrib);


	void check();
	void drawLine(glm::vec2 p0, glm::vec2 p1, float radius, Particle &particle);
	void perpendicular(Particle &particle, int x0, int y0, int dx, int dy, int p_error, float radius, int e, bool steep, int yStep);

public:
	//System();
//	System(float width, float height) : System(width, height, 0.1f) {};
	System(float width, float height, float& gridLength);
	//System(int width, int height, int initialNumOfParticles);
	~System();

	void update(double dt);
	float* getHeightMap();
};



#endif /* SYSTEM_NP_H_ */
