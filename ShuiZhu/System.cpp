#include "System.h"
#include <chrono>
#include <random>

#define GRAVITY 9.80665f

// WARNING: I assumed that +x points right, +y points down, but I feel like that is incorrect...

// System::System(){}

System::System(int width, int height) :
	WIDTH(width),
	HEIGHT(height),
	SIZE(width * height)
{
	id_map = new int[SIZE];
	height_map = new float[SIZE];
	affinity_map = new float[SIZE];

	// mersenne_twister_engine with seed from system clock
	//		(http://www.cplusplus.com/reference/random/mersenne_twister_engine/mersenne_twister_engine/)
	std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<int> distribution(1, 10000);
	for (int i = 0; i < SIZE; i++) {
		id_map[i] = -1;
		height_map[i] = 0.0f;
		// https://stackoverflow.com/questions/48716109/generating-a-random-number-between-0-1-c/48716227
		// https://codeforces.com/blog/entry/61587
		// https://onedrive.live.com/view.aspx?resid=E66E02DC83EFB165!312&cid=e66e02dc83efb165&lor=shortUrl
		affinity_map[i] = (float)distribution(generator) / 10000.0f ;		// random between [0,1]
	}

	// default to 10 water particles
	for (int i = 0;i < 10; i++) {
		particles.insert({ i, Particle(0,0) });
	}
}

System::~System() {
	delete [] id_map;
	delete [] height_map;
	delete [] affinity_map;
}

void System::update(double dt) {
	updateVelocity(dt);
	leaveResidualDroplets();
	updateHeightMap();
	mergeDroplets();
}

void System::updateVelocity(double dt) {
	// std::unordered_map<int, Particle>::iterator, for future me's curiosity
	for (auto i = particles.begin(); i != particles.end(); i++) {
		Particle p = i->second;

		float f_gravity = p.getMass() * GRAVITY;
		float f_friction = p.getMass_static() * GRAVITY;					// keep in mind that this assumes gravity points
		glm::vec2 acceleration = glm::vec2(0, f_gravity - f_friction) / p.getMass();		// straight down
		glm::vec2 velocity = p.getVelocity() + acceleration * (float)dt;
		float speed = glm::distance(velocity, glm::vec2(0,0));

	//	check 3 region's water amount, then affinity, then random
	// 		return random w (or speed * random w)
	// for you idiot, s*w = {(s*w.x), (s*w.y)}

		switch(determineDirectionOfMovement(p)) {
			case Region::BL :
				velocity = speed * glm::normalize(glm::vec2(-1, 1));
			case Region::B 	:
				velocity = speed * glm::vec2(0, 1);
			case Region::BR :
				velocity = speed * glm::normalize(glm::vec2(1, 1));
		}
		p.setVelocity(velocity);

	}
}

System::Region System::determineDirectionOfMovement(Particle p) {
	Region directionOfMovement = Region::BL;
	float max = 0.0f;

	// first check water amount
	float bl = sumOf(p.getPosition(), Region::BL, Attrib::water);
	float b = sumOf(p.getPosition(), Region::B, Attrib::water);
	if (bl > b) {
		max = bl;
		directionOfMovement = Region::BL;
	} else {
		max = b;
		directionOfMovement = Region::B;
	}
	float br = sumOf(p.getPosition(), Region::BR, Attrib::water);
	if (br > max) {
		max = br;
		directionOfMovement = Region::BR;
	}

	if (max > 0.0f)
		return directionOfMovement;

	// otherwise check affinities
	max = 0.0f;

	bl = sumOf(p.getPosition(), Region::BL, Attrib::affinity);
	b = sumOf(p.getPosition(), Region::B, Attrib::affinity);
	if (bl > b) {
		max = bl;
		directionOfMovement = Region::BL;
	} else {
		max = b;
		directionOfMovement = Region::B;
	}
	br = sumOf(p.getPosition(), Region::BR, Attrib::affinity);
	if (br > max) {
		max = br;
		directionOfMovement = Region::BR;
	}

	if (max > 0.0f)
		return directionOfMovement;

	// else, random direction
	std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<int> distribution(1, 3);
	int randomDOM = distribution(generator);
	switch(randomDOM) {
		case 1 :
			return Region::BL;
		case 2 :
			return Region::B;
		case 3 :
			return Region::BR;
	}

}

float System::sumOf(glm::ivec2 pos, Region region, Attrib attrib) {
	switch(region){
		case Region::BL :
			if (attrib == Attrib::water){
				return height_map[p(pos.x - 3, pos.y + 2)]
					+ height_map[p(pos.x - 2, pos.y + 2)]
					+ height_map[p(pos.x - 1, pos.y + 2)]
					+ height_map[p(pos.x - 3, pos.y + 3)]
					+ height_map[p(pos.x - 2, pos.y + 3)]
					+ height_map[p(pos.x - 1, pos.y + 3)]
					+ height_map[p(pos.x - 3, pos.y + 4)]
					+ height_map[p(pos.x - 2, pos.y + 4)]
					+ height_map[p(pos.x - 1, pos.y + 4)];
			} else {
				return affinity_map[p(pos.x - 3, pos.y + 2)]
					+ affinity_map[p(pos.x - 2, pos.y + 2)]
					+ affinity_map[p(pos.x - 1, pos.y + 2)]
					+ affinity_map[p(pos.x - 3, pos.y + 3)]
					+ affinity_map[p(pos.x - 2, pos.y + 3)]
					+ affinity_map[p(pos.x - 1, pos.y + 3)]
					+ affinity_map[p(pos.x - 3, pos.y + 4)]
					+ affinity_map[p(pos.x - 2, pos.y + 4)]
					+ affinity_map[p(pos.x - 1, pos.y + 4)];
			}
		case Region::B :
			if (attrib == Attrib::water){
				return height_map[p(pos.x - 1, pos.y + 2)]
					+ height_map[p(pos.x, 	  pos.y + 2)]
					+ height_map[p(pos.x + 1, pos.y + 2)]
					+ height_map[p(pos.x - 1, pos.y + 3)]
					+ height_map[p(pos.x, 	  pos.y + 3)]
					+ height_map[p(pos.x + 1, pos.y + 3)]
					+ height_map[p(pos.x - 1, pos.y + 4)]
					+ height_map[p(pos.x, 	  pos.y + 4)]
					+ height_map[p(pos.x + 1, pos.y + 4)];
			} else {
				return affinity_map[p(pos.x - 1, pos.y + 2)]
					+ affinity_map[p(pos.x, 	  pos.y + 2)]
					+ affinity_map[p(pos.x + 1, pos.y + 2)]
					+ affinity_map[p(pos.x - 1, pos.y + 3)]
					+ affinity_map[p(pos.x, 	  pos.y + 3)]
					+ affinity_map[p(pos.x + 1, pos.y + 3)]
					+ affinity_map[p(pos.x - 1, pos.y + 4)]
					+ affinity_map[p(pos.x, 	  pos.y + 4)]
					+ affinity_map[p(pos.x + 1, pos.y + 4)];
			}
		case Region::BR :
			if (attrib == Attrib::water){
				return height_map[p(pos.x + 1, pos.y + 2)]
					+ height_map[p(pos.x + 2, pos.y + 2)]
					+ height_map[p(pos.x + 3, pos.y + 2)]
					+ height_map[p(pos.x + 1, pos.y + 3)]
					+ height_map[p(pos.x + 2, pos.y + 3)]
					+ height_map[p(pos.x + 3, pos.y + 3)]
					+ height_map[p(pos.x + 1, pos.y + 4)]
					+ height_map[p(pos.x + 2, pos.y + 4)]
					+ height_map[p(pos.x + 3, pos.y + 4)];
			} else {
				return affinity_map[p(pos.x + 1, pos.y + 2)]
					+ affinity_map[p(pos.x + 2, pos.y + 2)]
					+ affinity_map[p(pos.x + 3, pos.y + 2)]
					+ affinity_map[p(pos.x + 1, pos.y + 3)]
					+ affinity_map[p(pos.x + 2, pos.y + 3)]
					+ affinity_map[p(pos.x + 3, pos.y + 3)]
					+ affinity_map[p(pos.x + 1, pos.y + 4)]
					+ affinity_map[p(pos.x + 2, pos.y + 4)]
					+ affinity_map[p(pos.x + 3, pos.y + 4)];
			}
	}
	return 0.0f;
}

void System::leaveResidualDroplets() {

}

void System::updateHeightMap() {
	constructNewHeightMap();
	smoothHeightMap();
	erodeHeightMap();
}
void System::constructNewHeightMap() {

}
void System::smoothHeightMap() {

}
void System::erodeHeightMap() {

}
void System::mergeDroplets() {

}

int System::p(glm::ivec2 pos) {
	return pos.y*WIDTH + pos.x;
}
int System::p(int x, int y) {
	return y*WIDTH + x;
}
