#include "System.h"
#include <chrono>
#include <random>
#include <iterator>
#include <algorithm>

#define GRAVITY 9.80665f
#define PI 3.14159265359f
#define BETA 3

// WARNING: I assumed that +x points right, +y points down, but I feel like that is incorrect...

// +x right, +y up, for pixels {1, 2, 3, 4} it is
// 3 4
// 1 2

// System::System(){}

System::System(float width, float height, float gridLength) :
	MAP_WIDTH((int)(width/gridLength)),
	MAP_HEIGHT((int)(height/gridLength)),
	MAP_SIZE(MAP_WIDTH * MAP_HEIGHT),
	GRID_LENGTH(gridLength)
{
	id_map = new int[MAP_SIZE];
	height_map = new float[MAP_SIZE];
	affinity_map = new float[MAP_SIZE];

	// mersenne_twister_engine with seed from system clock
	//		(http://www.cplusplus.com/reference/random/mersenne_twister_engine/mersenne_twister_engine/)
	std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<int> affinity_distribution(1, 10000);
	for (int i = 0; i < MAP_SIZE; i++) {
		id_map[i] = -1;
		height_map[i] = 0.0f;
		// https://stackoverflow.com/questions/48716109/generating-a-random-number-between-0-1-c/48716227
		// https://codeforces.com/blog/entry/61587
		// https://onedrive.live.com/view.aspx?resid=E66E02DC83EFB165!312&cid=e66e02dc83efb165&lor=shortUrl
		affinity_map[i] = (float)affinity_distribution(generator) / 10000.0f ;		// random between [0,1]
	}

	  int intervals[3] = {0, 500000, 1000000};
	  int weights[2] = {8, 2};
	  std::piecewise_constant_distribution<double>
	    mass_distribution (std::begin(intervals),std::end(intervals),std::begin(weights));
	  std::uniform_int_distribution<int> px_distribution(0, MAP_WIDTH - 1);
	  std::uniform_int_distribution<int> py_distribution(0, MAP_HEIGHT - 1);
	// default to 10 water particles
	for (int i = 0;i < 10; i++) {
		// approximately 80% of newly created masses are less than m_static
		float px = ((float)px_distribution(generator) + 0.5f) * GRID_LENGTH;
		float py = ((float)py_distribution(generator) + 0.5f) * GRID_LENGTH;
		float mass = mass_distribution(generator) / 10000000000.0f;
		particles.insert({ i, new Particle(glm::vec2(px, py), mass) });
		id_map[p(particles[i]->getPosition())] = i;

	}
}

System::~System() {
	delete [] id_map;
	delete [] height_map;
	delete [] affinity_map;
}

void System::update(double dt) {
	updateVelocity(dt);
	updatePosition(dt);
	leaveResidualDroplets(dt);
	updateHeightMap();
	mergeDroplets();
}

void System::updateVelocity(double dt) {
	// std::unordered_map<int, Particle>::iterator, for future me's curiosity
	for (auto i = particles.begin(); i != particles.end(); i++) {
		Particle* p = i->second;

		if (p->isStatic())
			continue;

		float f_gravity = p->getMass() * GRAVITY;
		float f_friction = p->getMass_static() * GRAVITY;					// keep in mind that this assumes gravity points
		glm::vec2 acceleration = glm::vec2(0, f_friction - f_gravity) / p->getMass();		// straight down
		glm::vec2 velocity = p->getVelocity() + acceleration * (float)dt;
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
		p->setVelocity(velocity);	// velocity is in reality
	}
}

System::Region System::determineDirectionOfMovement(Particle* p) {
	Region directionOfMovement = Region::BL;
	float max = 0.0f;
	glm::vec2 position = p->getPosition();

	// first check water amount
	float bl = sumOf(position, Region::BL, Attrib::water);
	float b = sumOf(position, Region::B, Attrib::water);
	if (bl > b) {
		max = bl;
		directionOfMovement = Region::BL;
	} else {
		max = b;
		directionOfMovement = Region::B;
	}
	float br = sumOf(position, Region::BR, Attrib::water);
	if (br > max) {
		max = br;
		directionOfMovement = Region::BR;
	}

	if (max > 0.0f)
		return directionOfMovement;

	// otherwise check affinities
	max = 0.0f;

	bl = sumOf(position, Region::BL, Attrib::affinity);
	b = sumOf(position, Region::B, Attrib::affinity);
	if (bl > b) {
		max = bl;
		directionOfMovement = Region::BL;
	} else {
		max = b;
		directionOfMovement = Region::B;
	}
	br = sumOf(position, Region::BR, Attrib::affinity);
	if (br > max) {
		max = br;
		directionOfMovement = Region::BR;
	}

	if (max > 0.0f)
		return directionOfMovement;

	// else, random direction
	std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<int> distribution(2, 4);
	Region randomDOM = (Region)distribution(generator);
	return randomDOM;
}

float System::sumOf(glm::vec2 position, Region region, Attrib attrib) {
	position = position / GRID_LENGTH;
	glm::ivec2 pos((int)position.x, (int)position.y);
	switch(region){
		case Region::BL :
			if (attrib == Attrib::water){
				return height_map[p(pos.x - 3, pos.y + 2)]		// turn to minus y, since +y is now up
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

void System::updatePosition(double dt) {
	for (auto i = particles.begin(); i != particles.end(); i++) {
		Particle* p = i->second;

		if (p->isStatic())
			continue;

		glm::vec2 position = p->getPosition() + p->getVelocity()*(float)dt;
		if ((int)position.x < 0)
			position.x += MAP_WIDTH * GRID_LENGTH;
		else if ((int)position.x >= MAP_WIDTH)
			position.x -= MAP_WIDTH * GRID_LENGTH;

		if ((int)position.x < 0)
			position.y += MAP_HEIGHT * GRID_LENGTH;
		else if ((int)position.x >= MAP_HEIGHT)
			position.y -= MAP_HEIGHT * GRID_LENGTH;

		p->setPosition(position);
	}
}

void System::leaveResidualDroplets(double dt) {

	/* for every moving particle (i.e., not static)
	 * if prob = leaveResidual() == true
	 * 		create new particle
	 * 		new particle.position = curr particle position
	 * 		new particle.mass = min(mass_static, random(0.1,0.3)*curr particle mass)
	 * 		curr particle mass = curr particle mass - new particle.mass
	 * 		new particle.velocity = (0,0)
	 * 		new particle.timeSinceLastResidual = 0
	 * 		curr particle.timeSinceLastResidual = 0
	 * 	else
	 * 		curr particle.timeSinceLastResidual += dt
	 */
	for (auto i = particles.begin(); i != particles.end(); i++) {
		Particle* p = i->second;
		if (p->isStatic())
			continue;			// do you need to reset residual time??
		if (p->leaveResidual(BETA, dt)) {
			std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
			std::uniform_int_distribution<int> distribution(100000, 300000);
			float a = distribution(generator) / 1000000.0f;
			float np_mass = std::min(p->getMass_static(), a * p->getMass());
			glm::vec2 np_position = p->getPosition();

			particles.insert({particles.size(), new Particle(np_position, np_mass)});
			p->setMass(p->getMass() - np_mass);
			p->resetTimeSinceLastResidual();

		} else {
			p->addResidualTime(dt);
		}
	}
}

void System::updateHeightMap() {
	assignDropletShapes();
	constructNewHeightMap();
	smoothHeightMap();
	erodeHeightMap();
}

void System::assignDropletShapes() {
	/* for particles
	 * 		if moving (not static)
	 * 			clear it's old q if necessary? or place this elsewhere...
	 * 			particle.radius = cubeRoot(root3) of (3*mass/(2*waterDensity*pi)), they set density to 1
	 * 		else
	 * 			particle.radius = cubeRoot((3*pi/4)*(2*mass/N)*waterDensity)
	 * 			if particle's current q.size == 0, it's newly static so
	 * 				N = [1,5]
	 * 				q[0] = particle.position
	 * 				for (i = 1; i < N; i++)
	 * 					direction = 4,5,6,7,8
	 *					q[i] = random(L, BL, B, BR, R) of q[i-1]
	 *			otherwise, the particle should have old q positions for reuse
	 */

	for (auto i = particles.begin(); i != particles.end(); i++) {
		Particle* p = i->second;

		if (p->isStatic()) {
			int n = p->getHemispherePositions().size();
			// if particle is newly static, fill in N[1,5] hemisphere positions
			// else, reuse previous positions
			if (n == 0) {
				std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
				std::uniform_int_distribution<int> distribution(1, 5);
				n = distribution(generator);
				glm::vec2 q(p->getPosition());
				p->addHemispherePosition(q);
				for (int i = 0; i < n - 1; i++) {
					Region direction = (Region)distribution(generator);
					switch(direction) {
						case Region::L :
							q += glm::vec2(-GRID_LENGTH, 0);
						case Region::BL :
							q += glm::vec2(-glm::sqrt(2), -glm::sqrt(2)) * GRID_LENGTH;
						case Region::B :
							q += glm::vec2(0, -GRID_LENGTH);
						case Region::BR :
							q += glm::vec2(glm::sqrt(2), -glm::sqrt(2)) * GRID_LENGTH;
						case Region::R :
							q += glm::vec2(GRID_LENGTH, 0);
					}
					p->addHemispherePosition(q);
				}
			}
			// particle.radius = cubeRoot((3*pi/4)*(2*mass/N)*waterDensity)
			float radius = glm::pow((3*PI/4) * (2*p->getMass()/n) * p->getDensity(), 1/3);
			p->setRadius(radius);
		} else {
			// if moving
			// particle.radius = cubeRoot(root3) of (3*mass/(2*waterDensity*pi)), they set density to 1
			p->clearHemispherePositions();
			float radius = glm::pow(3*p->getMass() / (2*p->getDensity()*PI), 1/3);
			p->setRadius(radius);
		}
	}
}

void System::constructNewHeightMap() {
	/* 	hmmmmmmm.....by particle, remember to update ID map as well
	 * get radius, drop decimal/convert to int, and search [p-r, p+r] on map
	 *  for [x - r, x + r]
	 *  	for [y - r, y + r]
	 *  		if radius^2 - ((x,y) - particle.position)^2 > 0
	 *  		and current height(x,y) < sqrt(radius^2 - ((x,y) - particle.position)^2)
	 *  			height = sqrt(radius^2 - ((x,y) - particle.position)^2)
	 *  			set ID
	 *  		else, continue
	 */

	for (auto i = particles.begin(); i != particles.end(); i++) {
		Particle* p = i->second;
	}
}
void System::smoothHeightMap() {
/*
 * height of every cell = height of (surrounding 8 neighbors + itself) / 9
 * if height < e_h, height = 0 --> e_h = 0.01
 */
}
void System::erodeHeightMap() {

}
void System::mergeDroplets() {

}

// for now, do wrap-around
int System::p(glm::vec2 pos) {
	// reality to grid
	glm::ivec2 p = pos / GRID_LENGTH;

	if (p.x < 0)
		p.x += MAP_WIDTH;
	else if (p.x >= MAP_WIDTH)
		p.x -= MAP_WIDTH;

	if (p.y < 0)
		p.y += MAP_HEIGHT;
	else if (p.y >= MAP_HEIGHT)
		p.y -= MAP_HEIGHT;

	return p.y*MAP_WIDTH + p.x;
}

int System::p(int x, int y) {
	if (x < 0)
		x += MAP_WIDTH;
	else if (x >= MAP_WIDTH)
		x -= MAP_WIDTH;

	if (y < 0)
		y += MAP_HEIGHT;
	else if (y >= MAP_HEIGHT)
		y -= MAP_HEIGHT;

	return y*MAP_WIDTH + x;
}
