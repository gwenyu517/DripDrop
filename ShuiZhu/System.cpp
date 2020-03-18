#include "System.h"
#include <chrono>
#include <random>

/*
class System {
private:
	const int WIDTH;
	const int HEIGHT;
	const int SIZE;

	int* id_map;
	float* height_map;	// or double??
	float* affinity_map; // [0,1] generate on construction
	std::unordered_map<int, Particle> particle;

	void updateVelocity();
	void leaveResidualDroplets();
	void updateHeightMap();
	void constructNewHeightMap();
	void smoothHeightMap();
	void erodeHeightMap();
	void mergeDroplets();

public:
	System();
	System(int width, int height);
	System(int width, int height, int initialNumOfParticles);
	~System();

	void update(float dt);
};
*/

// System::System(){}


System::System(int width, int height) :
	WIDTH(width),
	HEIGHT(height),
	SIZE(width * height)
{
	id_map = new int[SIZE];
	height_map = new float[SIZE];

	affinity_map = new float[SIZE];
	// mersenne_twister_engine with seed from system clock (http://www.cplusplus.com/reference/random/mersenne_twister_engine/mersenne_twister_engine/)
	std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<int> distribution(1, 10000);
	for (int i = 0; i < SIZE; i++) {
		// https://stackoverflow.com/questions/48716109/generating-a-random-number-between-0-1-c/48716227
		// https://codeforces.com/blog/entry/61587
		// https://onedrive.live.com/view.aspx?resid=E66E02DC83EFB165!312&cid=e66e02dc83efb165&lor=shortUrl
		affinity_map[i] = (float)distribution(generator) / 10000.0f ;		// random between [0,1]
	}

	// default to 10 water particles
	for (int i = 0;i < 10; i++) {
		particle.insert({ i, Particle() });
	}
}

System::~System() {
	delete [] id_map;
	delete [] height_map;
	delete [] affinity_map;
}

void System::update(float dt) {
	updateVelocity();
	leaveResidualDroplets();
	updateHeightMap();
	mergeDroplets();

}

void System::updateVelocity() {

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
