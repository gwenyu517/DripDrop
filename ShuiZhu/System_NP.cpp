#include "System_NP.h"
#include <chrono>
#include <random>
//#include <iterator>
#include <algorithm>

#include <cmath>	// std::nextafter, std::round
#include <iostream>	// for debug lol
#include <iomanip>


const float GRAVITY = 980.665f;	// cm/s^2
const float PI = 3.14159265359f;
const int BETA = 3;
const float E_H = 0.01;
const float MU = 1.5f;



// WARNING: I assumed that +x points right, +y points down, but I feel like that is incorrect...

// +x right, +y up, for pixels {1, 2, 3, 4} it is
// 3 4
// 1 2

// System::System(){}

unsigned long affinitySeed = 2117095920;

System::System(float width, float height, float& gridLength) :
//	MAP_WIDTH((int)(width/gridLength)),
//	MAP_HEIGHT((int)(height/gridLength)),
	GRID_LENGTH(gridLength > 0.1f ? 0.1f : gridLength),		// max gridLength = 0.1 cm
	MAP_WIDTH((int)std::round((width/GRID_LENGTH))),
	MAP_HEIGHT((int)std::round((height/GRID_LENGTH))),
	MAP_SIZE(MAP_WIDTH * MAP_HEIGHT)
{
	gridLength = GRID_LENGTH;
	std::cout << std::setprecision(16);
	//id_map = new int[MAP_SIZE];
	height_map = new float[MAP_SIZE];
	affinity_map = new float[MAP_SIZE];

	std::cout << "GL = " << GRID_LENGTH << std::endl;
	std::cout << "WIDTH = " << MAP_WIDTH << "; HEIGHT = " << MAP_HEIGHT << std::endl;

	// mersenne_twister_engine with seed from system clock
	//		(http://www.cplusplus.com/reference/random/mersenne_twister_engine/mersenne_twister_engine/)
	//affinitySeed = std::chrono::system_clock::now().time_since_epoch().count();
//	std::mt19937 generator(affinitySeed);
	std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_real_distribution<float> affinity_distribution(0.0, std::nextafter(1.0,2.0));
	for (int i = 0; i < MAP_SIZE; i++) {
		id_map.push_back( std::vector<int>(1,-1) );	// push back 1 element = -1
		height_map[i] = 0.0f;
		// https://stackoverflow.com/questions/48716109/generating-a-random-number-between-0-1-c/48716227
		// https://codeforces.com/blog/entry/61587
		// https://onedrive.live.com/view.aspx?resid=E66E02DC83EFB165!312&cid=e66e02dc83efb165&lor=shortUrl
//		affinity_map[i] = 0.5f;
		affinity_distribution(generator);	// random between [0,1]
	}

	  // static, N=5
	  minDropletMass = glm::pow(0.0225*0.0225 + GRID_LENGTH/glm::pow(2.0,0.5), 1.5f) * 10.0f / (3.0f * PI * Particle::getDensity());

	  // moving, r = 0.6
//	  maxDropletMass = 0.045;
	  maxDropletMass = 0.03;

	  // mass_static --> N=1 gives r = 0.509096..., N=5 gives r = 0.297721...
	  Particle::setMass_static(0.028f);	// plot twist: float precision problems, 0.028 is actually 0.028......864267....etc.
	  //std::cout << "stat = " << Particle::getMass_static() << std::endl;

//	  particleList.insert({Particle::getNextID(), Particle(glm::vec2(width/2.0f, 0.75*height), 0.02)});
//	  particleList.insert({Particle::getNextID(), Particle(glm::vec2(0.5f*width, 0.825f*height), 0.03)});


	  particleList.insert({Particle::getNextID(), Particle(glm::vec2(0.5f*width, 0.5f*height), 0.03)});

//	  std::cout << "set up position = " << px << ", " << py << " --> " << index(glm::vec2(px,py)) << std::endl;
//	  std::cout << "isStatic = " << (particleList.begin()->second)->isStatic() << std::endl;
//	  std::cout << "mass ? " << (particleList.begin()->second)->getMass() << std::endl;


	  std::cout << "start???" << std::endl;

	  update(0.0);
}

System::~System() {
	std::cout << "Destructor" << std::endl;
//	delete [] id_map;
	delete [] height_map;
	delete [] affinity_map;
}

float* System::getHeightMap() {
	return height_map;
}

void System::update(double dt) {
	std::cout << "___________" << affinitySeed << "____________________" << std::endl;

	std::cout << "dtboi: " << dt << std::endl;
	//generateParticles();
	updateVelocity(dt);
	updatePosition(dt);
	leaveResidualDroplets(dt);
	updateHeightMap();
	deleteOutOfBoundDroplets();
	mergeDroplets();
	std::cout << "DONE" << std::endl;
//	check();
}

void System::check() {
	for (int i = 0; i < MAP_WIDTH; i++) {
		for (int j = 0; j < MAP_HEIGHT; j++) {
			if (height_map[index(i,j)] > 0.0f)
				std::cout << i << ", " << j << " = " << height_map[index(i,j)] << std::endl;
		}
	}
}

void System::generateParticles() {
	std::cout << "GENERATION" << std::endl;
	std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());

	// approximately 80% of newly created masses are less than m_static
	float intervals[3] = {minDropletMass, Particle::getMass_static(), maxDropletMass};
	int weights[2] = {8, 2};
	std::piecewise_constant_distribution<float>
		mass_distribution (&intervals[0],&intervals[2],&weights[0]);

	std::uniform_real_distribution<float> px_distribution(0.0, std::nextafter(MAP_WIDTH*GRID_LENGTH,MAP_WIDTH*GRID_LENGTH + 1.0));
	std::uniform_real_distribution<float> py_distribution(0.0, std::nextafter(MAP_HEIGHT*GRID_LENGTH,MAP_HEIGHT*GRID_LENGTH + 1.0));

	std::uniform_int_distribution<int> numberOfParticles_distribution(0,10);

	for (int i = 0;i < numberOfParticles_distribution(generator); i++) {
		float px = px_distribution(generator);
		float py = py_distribution(generator);
		std::cout << "  generate at " << px << ", " << py << std::endl;
		float mass = mass_distribution(generator);
//		std::cout << "    with mass " << mass << std::endl;
		particleList.insert({ Particle::getNextID(), Particle(glm::vec2(px, py), mass) });
//		id_map[index(px,py)] = i;				// might not be necessary
	}
	std::cout << " ~ we currently have " << particleList.size() << " particles " << std::endl;
}

void System::updateVelocity(double dt) {
	// std::unordered_map<int, Particle>::iterator, for future me's curiosity
	std::cout << "UPDATE VELOCITY " << std::endl;
	for (auto i = particleList.begin(); i != particleList.end(); i++) {
		Particle &p = i->second;
//		std::cout << "particle " << p.getID() << " " << p.getMass() << std::endl;

		if (p.isStatic())
			continue;

//		std::cout << "mass_static = " << p.getMass_static() << std::endl;
		float f_gravity = p.getMass() * GRAVITY;
		float f_friction = p.getMass_static() * GRAVITY;					// keep in mind that this assumes gravity points
//		std::cout << "fg = " << f_gravity << ", ff = " << f_friction << std::endl;
		glm::vec2 acceleration = glm::vec2(0, f_friction - f_gravity) / p.getMass();		// straight down
		std::cout << "  " << p.getID() << " acceleration = " << acceleration.x << ", " << acceleration.y << std::endl;
		glm::vec2 velocity = p.getVelocity() + acceleration * (float)dt;
//		std::cout << "velocity waaaas = " << p.getVelocity().x << ", " << p.getVelocity().y << std::endl;
//		std::cout << "acceleration * v.x? " << (acceleration.x * p.getVelocity().x) << std::endl;
		std::cout << "        velocity = " << velocity.x << ", " << velocity.y << std::endl;
		float speed = glm::length(velocity);

	//	check 3 region's water amount, then affinity, then random
	// 		return random w (or speed * random w)
	// for you idiot, s*w = {(s*w.x), (s*w.y)}

		switch(determineDirectionOfMovement(p)) {
			case Region::BL :
				std::cout << "       BL" << std::endl;
				velocity = speed * glm::normalize(glm::vec2(-1, -1));
				break;
			case Region::B 	:
				std::cout << "       B" << std::endl;
				velocity = speed * glm::vec2(0, -1);
				break;
			case Region::BR :
				std::cout << "       BR" << std::endl;
				velocity = speed * glm::normalize(glm::vec2(1, -1));
				break;
			default:
				break;
		}
		//p->setVelocity(glm::vec2(0,0));
//		p.setVelocity(velocity*0.8f);
		p.setVelocity(velocity);	// velocity is in reality
//		std::cout << "("<< velocity.x << ", " << velocity.y << ")";

	}
	//std::cout << " **************************** I HAVE " << particleList.size() << " PARTICLES" << std::endl;
}


System::Region System::determineDirectionOfMovement(Particle p) {
	Region directionOfMovement = Region::BL;
	float max = 0.0f;
	glm::vec2 position = p.getPosition();

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
//	std::cout << "     bl = " << bl << std::endl;
//	std::cout << "     b  = " << b << std::endl;
//	std::cout << "     br = " << br << std::endl;

//	std::cout << "dom " << (int)directionOfMovement << ", with sum " << max << std::endl;
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

//	std::cout << "dom " << (int)directionOfMovement << ", with affinity " << max << std::endl;

	if (max > 0.0f)
		return directionOfMovement;

	// else, random direction
	std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<int> distribution(2, 4);
	Region randomDOM = (Region)distribution(generator);
//	std::cout << "direction " << (int)randomDOM << std::endl;
	return randomDOM;
}


float System::sumOf(glm::vec2 position, Region region, Attrib attrib) {
	position = position / GRID_LENGTH;
	glm::ivec2 pos((int)position.x, (int)position.y);
	switch(region){
		case Region::BL :
			if (attrib == Attrib::water){
				return height_map[index(pos.x - 3, pos.y - 2)]		// turn to minus y, since +y is now up
					+ height_map[index(pos.x - 2, pos.y - 2)]
					+ height_map[index(pos.x - 1, pos.y - 2)]
					+ height_map[index(pos.x - 3, pos.y - 3)]
					+ height_map[index(pos.x - 2, pos.y - 3)]
					+ height_map[index(pos.x - 1, pos.y - 3)]
					+ height_map[index(pos.x - 3, pos.y - 4)]
					+ height_map[index(pos.x - 2, pos.y - 4)]
					+ height_map[index(pos.x - 1, pos.y - 4)];
			} else {
				return affinity_map[index(pos.x - 3, pos.y - 2)]
					+ affinity_map[index(pos.x - 2, pos.y - 2)]
					+ affinity_map[index(pos.x - 1, pos.y - 2)]
					+ affinity_map[index(pos.x - 3, pos.y - 3)]
					+ affinity_map[index(pos.x - 2, pos.y - 3)]
					+ affinity_map[index(pos.x - 1, pos.y - 3)]
					+ affinity_map[index(pos.x - 3, pos.y - 4)]
					+ affinity_map[index(pos.x - 2, pos.y - 4)]
					+ affinity_map[index(pos.x - 1, pos.y - 4)];
			}
			break;
		case Region::B :
			if (attrib == Attrib::water){
				return height_map[index(pos.x - 1, pos.y - 2)]
					+ height_map[index(pos.x, 	  pos.y - 2)]
					+ height_map[index(pos.x + 1, pos.y - 2)]
					+ height_map[index(pos.x - 1, pos.y - 3)]
					+ height_map[index(pos.x, 	  pos.y - 3)]
					+ height_map[index(pos.x + 1, pos.y - 3)]
					+ height_map[index(pos.x - 1, pos.y - 4)]
					+ height_map[index(pos.x, 	  pos.y - 4)]
					+ height_map[index(pos.x + 1, pos.y - 4)];
			} else {
				return affinity_map[index(pos.x - 1, pos.y - 2)]
					+ affinity_map[index(pos.x, 	  pos.y - 2)]
					+ affinity_map[index(pos.x + 1, pos.y - 2)]
					+ affinity_map[index(pos.x - 1, pos.y - 3)]
					+ affinity_map[index(pos.x, 	  pos.y - 3)]
					+ affinity_map[index(pos.x + 1, pos.y - 3)]
					+ affinity_map[index(pos.x - 1, pos.y - 4)]
					+ affinity_map[index(pos.x, 	  pos.y - 4)]
					+ affinity_map[index(pos.x + 1, pos.y - 4)];
			}
			break;
		case Region::BR :
			if (attrib == Attrib::water){
				return height_map[index(pos.x + 1, pos.y - 2)]
					+ height_map[index(pos.x + 2, pos.y - 2)]
					+ height_map[index(pos.x + 3, pos.y - 2)]
					+ height_map[index(pos.x + 1, pos.y - 3)]
					+ height_map[index(pos.x + 2, pos.y - 3)]
					+ height_map[index(pos.x + 3, pos.y - 3)]
					+ height_map[index(pos.x + 1, pos.y - 4)]
					+ height_map[index(pos.x + 2, pos.y - 4)]
					+ height_map[index(pos.x + 3, pos.y - 4)];
			} else {
				return affinity_map[index(pos.x + 1, pos.y - 2)]
					+ affinity_map[index(pos.x + 2, pos.y - 2)]
					+ affinity_map[index(pos.x + 3, pos.y - 2)]
					+ affinity_map[index(pos.x + 1, pos.y - 3)]
					+ affinity_map[index(pos.x + 2, pos.y - 3)]
					+ affinity_map[index(pos.x + 3, pos.y - 3)]
					+ affinity_map[index(pos.x + 1, pos.y - 4)]
					+ affinity_map[index(pos.x + 2, pos.y - 4)]
					+ affinity_map[index(pos.x + 3, pos.y - 4)];
			}
			break;
		default:
			break;
	}
	return 0.0f;
}


void System::updatePosition(double dt) {
	std::cout <<"UPDATE POSITION " << std::endl;
	for (auto i = particleList.begin(); i != particleList.end(); i++) {
		Particle &p = i->second;

		if (p.isStatic())
			continue;

		glm::vec2 position = p.getPosition() + p.getVelocity()*(float)dt;

		//std::cout << "\nNEW POSITION" << std::endl;
		std::cout << "  " << p.getID() << " at " << position.x << ", " << position.y << std::endl;
		std::cout << "              velocity = " << p.getVelocity().x << ", " << p.getVelocity().y << std::endl;
		p.setPosition(position);
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
	std::cout << "LEAVE RESIDUAL?" << std::endl;
	for (auto i = particleList.begin(); i != particleList.end(); i++) {
		Particle &p = i->second;
		if (p.getMass() > maxDropletMass)
			std::cout << p.getID() << " is fat!! " << p.getMass() << std::endl;
//		std::cout << p.getID() << " ";
		if (p.isStatic()) {
//			std::cout << "        skip " << p.getID() << std::endl;
//			p.addResidualTime(dt);	// necessary to differentiate static vs residual?
			continue;			// do you need to reset residual time??
		}
		std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
		std::uniform_real_distribution<float> chance(0.0, std::nextafter(1.0,2.0));
		if ( p.leaveResidual(dt, chance(generator)) ) {
//			std::cout << " yes " << std::endl;
//			std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
			std::uniform_real_distribution<float> distribution(0.1, std::nextafter(0.3,1.0));
			float a = distribution(generator);
//			std::cout << "a = " << a << std::endl;
			float np_mass = std::min(p.getMass_static(), a * p.getMass());
			glm::vec2 np_position = p.getPosition();

			int np_ID = Particle::getNextID();
			particleList.insert({ np_ID, Particle(np_position, np_mass) });
			particleList[np_ID].setParent(p.getID());
			std::cout << "  " << p.getID() << " left " << np_ID << " at " << np_position.x << ", " << np_position.y << std::endl;

			p.setMass(p.getMass() - np_mass);
			p.resetTimeSinceLastResidual();
			//std::cout << "time since residual " << p->getTimeSinceLastResidual() << std::endl;

		} else {
			p.addResidualTime(dt);
//			std::cout << "        " << p.getID() << " time since residual " << p.getTimeSinceLastResidual() << std::endl;
		}
	}
}


void System::updateHeightMap() {
	// something wrong with erode();
	assignDropletShapes();
	constructNewHeightMap();
	smoothHeightMap();
	erodeHeightMap();
	std::cout << "End of updateHeightBoi" << std::endl;
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

	std::cout << "ASSIGN DROPLET SHAPE" << std::endl;

	for (auto i = particleList.begin(); i != particleList.end(); i++) {
		Particle &p = i->second;
//		std::cout << "is static? " << p.isStatic() << std::endl;
		//std::cout << "mass = " << p->getMass() << std::endl;

		if (p.isStatic()) {
			int n = p.getHemispherePositions().size();
//			std::cout << "n = " << n << " sphere positions" << std::endl;
			//std::cout << "number of existing sphere positions = " << n << std::endl;
			// if particle is newly static, fill in N[1,5] hemisphere positions
			// else, reuse previous positions
			if (n == 0) {
				std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
				std::uniform_int_distribution<int> distribution(1, 5);
				n = distribution(generator);
//				n = 5;
//				std::cout << "n = " << n << " sphere positions" << std::endl;
				glm::vec2 q(p.getPosition());
				p.addHemispherePosition(q);

//				/*** flicker debug ***/
//				for (int i = 0; i < n - 1; i++) {
//					Region direction = Region::B;
//					if (i == 0)
//						direction = (Region)3;
//					else if (i == 1)
//						direction = (Region)4;
//					else if (i == 2)
//						direction = (Region)3;
//					else if (i == 3)
//						direction = (Region)2;
//					switch(direction) {
//					case Region::L :
//						q += glm::vec2(-GRID_LENGTH, 0);
//						break;
//					case Region::BL :
//						q += glm::vec2(-glm::sqrt(2), -glm::sqrt(2)) * GRID_LENGTH;
//						break;
//					case Region::B :
//						q += glm::vec2(0, -GRID_LENGTH);
//						break;
//					case Region::BR :
//						q += glm::vec2(glm::sqrt(2), -glm::sqrt(2)) * GRID_LENGTH;
//						break;
//					case Region::R :
//						q += glm::vec2(GRID_LENGTH, 0);
//						break;
//					default:
//						break;
//					}
//					p.addHemispherePosition(q);
//				}
//
//				/*** end ***/

				/*** original code ***/
				for (int i = 0; i < n - 1; i++) {
					Region direction = (Region)distribution(generator);
//					std::cout << "sphere at " << (int)direction << std::endl;
					switch(direction) {
						case Region::L :
							q += glm::vec2(-GRID_LENGTH, 0);
							break;
						case Region::BL :
							q += glm::vec2(-glm::sqrt(2), -glm::sqrt(2)) * GRID_LENGTH;
							break;
						case Region::B :
							q += glm::vec2(0, -GRID_LENGTH);
							break;
						case Region::BR :
							q += glm::vec2(glm::sqrt(2), -glm::sqrt(2)) * GRID_LENGTH;
							break;
						case Region::R :
							q += glm::vec2(GRID_LENGTH, 0);
							break;
						default:
							break;
					}
//					std::cout << "add position " << q.x << ", " << q.y << std::endl;
					p.addHemispherePosition(q);
				}
				/**** end ***/
			}
			// particle.radius = cubeRoot((3*pi/4)*(2*mass/N)*waterDensity)
			float radius = glm::pow(1.5f*PI*p.getMass()*p.getDensity() / n, 1.0/3.0);

			// below is moving radius; just to check
			//float radius = glm::pow(3*p->getMass() / (2*p->getDensity()*PI), 1.0/3.0);
//			std::cout << "radius = " << radius << std::endl;
			p.setRadius(radius);
		} else {
			// if moving
			//std::cout << "moving" << std::endl;
			// particle.radius = cubeRoot(root3) of ((3/2)*mass*waterDensity*pi)), they set density to 1
			p.clearHemispherePositions();
			float radius = glm::pow(1.5f*PI*p.getMass()*p.getDensity(), 1.0/3.0);
//			std::cout << "radius = " << radius << std::endl;
			p.setRadius(radius);
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
	std::cout << "CONSTRUCT HEIGHT MAP" << std::endl;

	// for debug, clear height map every time
//	for (int i = 0; i < MAP_WIDTH; i++) {
//		for (int j = 0; j < MAP_HEIGHT; j++) {
//			height_map[index(i,j)] = 0.0f;
//		}
//	}

	for (auto p = particleList.begin(); p != particleList.end(); p++) {
		Particle &particle = p->second;

		// for debug
//		glm::vec2 particle_position = particle.getPosition();

//		std::cout << particle.getID() << " " << particle_position.x << ", " << particle_position.y;
//		std::cout << " --> " << index(particle_position) << std::endl;
//		float r = particle.getRadius();

		// YOU FORGOT TO CHECK HEMISPHERE POSITIONS
		//std::cout << "position = " << particle_position.x << ", " << particle_position.y << std::endl;
		//std::cout << "radius = " << r << std::endl;

		std::vector<glm::vec2> hemispherePositions = particle.getHemispherePositions();
		int n = hemispherePositions.size();

		// if moving particle
		if (n == 0) {
			glm::vec2 particle_position = particle.getPosition();
			placeHemisphere(particle, particle_position);

			glm::vec2 particlePrevPosition = particle.getPrevPosition();

			if (glm::distance(particle_position, particlePrevPosition) > GRID_LENGTH){
//				std::cout << "  draw line" << std::endl;
				glm::ivec2 prevPosition = gridPosition(particlePrevPosition);
				glm::ivec2 currPosition = gridPosition(particle_position);
				if (!isOutOfBounds(prevPosition.x, prevPosition.y) || !isOutOfBounds(currPosition.x, currPosition.y))
					drawLine(particle_position, particle.getPrevPosition(), particle.getRadius(), particle);


//				std::cout << "  done" << std::endl;
			}
		}
		else { // static
			//std::cout << "particle position (" << particle->getPosition().x << ", " << particle->getPosition().y << ")" << std::endl;
			for (int h = 0; h < n; h++) {
				glm::vec2 sphere_position = hemispherePositions[h];
				placeHemisphere(particle, sphere_position);
			}
		}
	}
	//std::cout << "NEW HEIGHT MAP MADE" << std::endl;
}

void System::placeHemisphere(Particle &particle, glm::vec2 hemispherePosition) {
	float r = particle.getRadius();
	int pID = particle.getID();

	int x0 = (int)((hemispherePosition.x - r) / GRID_LENGTH);
	int x1 = (int)((hemispherePosition.x + r) / GRID_LENGTH);
	int y0 = (int)((hemispherePosition.y - r) / GRID_LENGTH);
	int y1 = (int)((hemispherePosition.y + r) / GRID_LENGTH);



	for (int i = x0; i <= x1; i++) {
		for (int j = y0; j <= y1; j++) {

			// if the cell is out of bounds, don't "draw" it
			if (isOutOfBounds(i,j))
				continue;

			// h in actual terms
			float h = r*r - glm::distance(position(i,j), hemispherePosition)*glm::distance(position(i,j), hemispherePosition);
			if (h > 0 && height_map[index(i,j)] < glm::sqrt(h)){
				//std::cout << "h " << index(i,j) << " " << glm::sqrt(h) << std::endl;
				height_map[index(i,j)] = glm::sqrt(h);
				particle.addOccupiedCells(index(i,j));

				if (id_map[index(i,j)][0] == -1)
					id_map[index(i,j)][0] = pID;
				else if (std::count(id_map[index(i,j)].begin(), id_map[index(i,j)].end(), pID) == 0)
					id_map[index(i,j)].push_back(pID);
			}
		}
	}
}

void System::drawLine(glm::vec2 p0, glm::vec2 p1, float radius, Particle &particle) {
	std::cout << "  line from " << p0.x << ", " << p0.y << " to " << p1.x << ", " << p1.y << std::endl;
	int x0 = (int)(p0.x / GRID_LENGTH);
	int x1 = (int)(p1.x / GRID_LENGTH);
	int y0 = (int)(p0.y / GRID_LENGTH);
	int y1 = (int)(p1.y / GRID_LENGTH);

	bool steep = (abs(y1-y0) > abs(x1-x0)) ? true : false;
	if (steep) {	// do calculations as if it were in the quadrants nearest x-axis
		int temp = x0;
		x0 = y0;
		y0 = temp;

		temp = x1;
		x1 = y1;
		y1 = temp;
	}
	if (x0 > x1) {	// do calculations as if x were increasing, instead of decreasing
		int temp = x0;
		x0 = x1;
		x1 = temp;

		temp = y0;
		y0 = y1;
		y1 = temp;
	}

	int dy = abs(y1-y0);
	int yStep = y0 > y1 ? -1 : 1;
	int dx = x1 - x0; // no need to do abs because of if/swap
	int error = 0;
	int p_error = 0;
	int y = y0;


	for (int x = x0; x <= x1; x++) {
//		if (steep)
//			height_map[index(y,x)] = 1.0f;
//		else
//			height_map[index(x,y)] = 1.0f;
		perpendicular(particle, x, y, dx, dy, p_error, radius, error, steep, yStep);
		if (error > dx - 2*dy) {
			y += yStep;
			error += -2*dx;
			if (p_error > dx - 2*dy) {
				perpendicular(particle, x, y, dx, dy, p_error - 2*dx + 2*dy, radius, error, steep, yStep);
				p_error += -2*dx;
			}
			p_error += 2*dy;
		}
		error += 2*dy;
	}
}

void System::perpendicular(Particle &particle, int x0, int y0, int dx, int dy, int p_error, float radius, int e, bool steep, int yStep) {
	int x = x0;
	int y = y0;
	int error = p_error;
	int curr_thickness = dx + dy - e;


	int w = (std::ceil(radius/GRID_LENGTH)) * 2 * glm::pow(dx*dx + dy*dy, 0.5);  // scale width by 2*D

//	float h = radius*radius;

	while (curr_thickness <= w) {
		if (steep) {
			if (!isOutOfBounds(y,x)) {
				float h = radius*radius - glm::distance(position(y,x), position(y0,x0))*glm::distance(position(y,x), position(y0,x0));
				if (h > 0 && height_map[index(y,x)] < glm::sqrt(h)){
					height_map[index(y,x)] = glm::sqrt(h);
					particle.addOccupiedCells(index(y,x));

					if (id_map[index(y,x)][0] == -1)
						id_map[index(y,x)][0] = particle.getID();
					else if (std::count(id_map[index(y,x)].begin(), id_map[index(y,x)].end(), particle.getID()) == 0)
						id_map[index(y,x)].push_back(particle.getID());
				}
			}
		}
		else {
			if (!isOutOfBounds(x,y)) {
				float h = radius*radius - glm::distance(position(x,y), position(x0,y0))*glm::distance(position(x,y), position(x0,y0));
				if (h > 0 && height_map[index(x,y)] < glm::sqrt(h)){
					height_map[index(x,y)] = glm::sqrt(h);
					particle.addOccupiedCells(index(x,y));

					if (id_map[index(x,y)][0] == -1)
						id_map[index(x,y)][0] = particle.getID();
					else if (std::count(id_map[index(x,y)].begin(), id_map[index(x,y)].end(), particle.getID()) == 0)
						id_map[index(x,y)].push_back(particle.getID());
				}
			}
		}
		if (error > dx - 2*dy) {
			x += -1;
			error += -2*dx;
			curr_thickness += 2*dy;
		}
		y += yStep;
		error += 2*dy;
		curr_thickness += 2*dx;
//		std::cout << "new stuffs " << x << ", " << y << std::endl;
	}

	x = x0;
	y = y0;
	error = -p_error;
	curr_thickness = dx + dy + e;

	while (curr_thickness <= w) {
		if (steep) {
			if (!isOutOfBounds(y,x)) {
				float h = radius*radius - glm::distance(position(y,x), position(y0,x0))*glm::distance(position(y,x), position(y0,x0));
				if (h > 0 && height_map[index(y,x)] < glm::sqrt(h)){
					height_map[index(y,x)] = glm::sqrt(h);
					particle.addOccupiedCells(index(y,x));

					if (id_map[index(y,x)][0] == -1)
						id_map[index(y,x)][0] = particle.getID();
					else if (std::count(id_map[index(y,x)].begin(), id_map[index(y,x)].end(), particle.getID()) == 0)
						id_map[index(y,x)].push_back(particle.getID());
				}
			}
		}
		else {
			if (!isOutOfBounds(x,y)) {
				float h = radius*radius - glm::distance(position(x,y), position(x0,y0))*glm::distance(position(x,y), position(x0,y0));
				if (h > 0 && height_map[index(x,y)] < glm::sqrt(h)){
					height_map[index(x,y)] = glm::sqrt(h);
					particle.addOccupiedCells(index(x,y));

					if (id_map[index(x,y)][0] == -1)
						id_map[index(x,y)][0] = particle.getID();
					else if (std::count(id_map[index(x,y)].begin(), id_map[index(x,y)].end(), particle.getID()) == 0)
						id_map[index(x,y)].push_back(particle.getID());
				}
			}
		}
		if (error > dx - 2*dy) {
			x += 1;
			error += -2*dx;
			curr_thickness += 2*dy;
		}
		y += -yStep;
		error += 2*dy;
		curr_thickness += 2*dx;
	}

}

bool System::isOutOfBounds(int i, int j) {
	if (i < 0)
		return true;
	else if (i > MAP_WIDTH - 1)
		return true;
	else if (j < 0)
		return true;
	else if (j > MAP_HEIGHT - 1)
		return true;

	return false;
}

void System::smoothHeightMap() {
/*
 * height of every cell = height of (surrounding 8 neighbors + itself) / 9
 * if height < e_h, height = 0 --> e_h = 0.01
 */
	std::cout <<"SMOOTH SON, SMOOTH" << std::endl;
	float* h_map = new float[MAP_SIZE];
//	std::cout << "Initialized h_map" << std::endl;

	for (int i = 0; i < MAP_WIDTH; i++) {
		for (int j = 0; j < MAP_HEIGHT; j++) {

//			h_map[index(i,j)] = height_map[index(i,j)];
			float sum = height_map[index(i-1, j+1)] + height_map[index(i, j+1)] + height_map[index(i+1, j+1)]
					  + height_map[index(i-1, j)]   + height_map[index(i, j)]   + height_map[index(i+1, j)]
					  + height_map[index(i-1, j-1)] + height_map[index(i, j-1)] + height_map[index(i+1, j-1)];
//			std::cout << "Sum" << std::endl;
//			std::cout << "index hmap" << (index(i, j) >= MAP_SIZE) << std::endl;
			h_map[index(i,j)] = sum / 9.0f;

			if (h_map[index(i,j)] < E_H) {
//				if (h_map[index(i,j)] > 0.0f)
					//std::cout << "died " << index(i,j) << " " << h_map[index(i,j)] << std::endl;
				h_map[index(i,j)] = 0.0f;

				if (id_map[index(i,j)][0] != -1) {
//					std::cout << "idmap" << (index(i, j) >= MAP_SIZE) << std::endl;
					//std::cout << "died " << h_map[index(i,j)] << std::endl;
					for (int k = 0; k < id_map[index(i,j)].size(); k++){
						int id = id_map[index(i,j)][k];
						particleList[id].removeOccupiedCells(index(i,j));
					}
					id_map[index(i,j)].clear();
					id_map[index(i,j)].push_back(-1);
				}
			}
//			std::cout << "indexed idmap" << std::endl;
		}
	}

	delete height_map;
	height_map = h_map;

//	check();
	//std::cout << "DONE SMOOTH" << std::endl;
}

void System::erodeHeightMap() {
/*
 * if boundary grid cell (h(x,y) > 0 while neighboring cell has height = 0)
 * 		if in residual droplet
 * 			no change
 * 		if not in residual droplet
 * 			replace neighboring cell with greatest height with the boundary grid's
 * 			height value
 * 			change boundary grid height to 0
 */
	std::cout << "ERODE" << std::endl;
	float* h_map = new float[MAP_SIZE];
	for (int i = 0; i < MAP_SIZE; i++) {
		h_map[i] = height_map[i];
	}

	for (int i = 0; i < MAP_WIDTH; i++) {
		for (int j = 0; j < MAP_HEIGHT; j++) {

			std::vector<int> &id_cell = id_map[index(i,j)];

			// if the particle is static/a residual, skip
//			if (id != -1 && particleList[id].isStatic()) {
//				//std::cout << "skip " << i << ", " << j << std::endl;
//				continue;
//			}

			// if cell lies in region of residual droplet, skip
			if (isResidualDropletCell(i,j))
				continue;

			// if is boundary cell and not in residual droplet
			else if (isBoundaryCell(i,j)) {
//				std::cout << "die " << i << ", " << j << " --> ";

				// determine grid cell to shift to
				glm::vec2 gradient( height_map[index(i+1, j+1)] + height_map[index(i+1, j)] + height_map[index(i+1, j-1)]
								 - (height_map[index(i-1, j+1)] + height_map[index(i-1, j)] + height_map[index(i-1, j-1)])
								,
								 height_map[index(i-1, j+1)] + height_map[index(i, j+1)] + height_map[index(i+1, j+1)]
								 - (height_map[index(i-1, j-1)] + height_map[index(i, j-1)] + height_map[index(i+1, j-1)]) );


				// angle = arccos( (gradient * [1,0]) / (|gradient||[1,0]|) )
				// angle = glm::acos( glm::normalize(gradient).x );
				float angle = glm::acos( gradient.x/glm::length(gradient) );

				int innerGrid;
				int m;

				if (gradient.y > 0) // angle = [0, pi]
					m = 1;
				else // angle = [pi, 2pi]
					m = -1;

				// ggh is purely for debug
				glm::ivec2 ggh;
				if (angle < PI/8) {
					innerGrid = index(i+1,j);
					ggh = glm::ivec2(i+1,j);
				}
				else if (angle < 3*PI/8){
					innerGrid = index(i+1, j+m);
					ggh = glm::ivec2(i+1,j+m);
				}
				else if (angle < 5*PI/8){
					innerGrid = index(i, j+m);
					ggh = glm::ivec2(i,j+m);
				}
				else if (angle < 7*PI/8){
					innerGrid = index(i-1, j+m);
					ggh = glm::ivec2(i-1,j+m);
				}
				else{
					innerGrid = index(i-1, j);
					ggh = glm::ivec2(i-1,j);
				}


//				std::cout << "ggh = " << ggh.x << ", " << ggh.y << std::endl;

				// then change height values
				h_map[innerGrid] = height_map[index(i,j)];
				h_map[index(i,j)] = 0.0f;
				// remove ids at boundary cell
				if (id_cell[0] != -1) {
					for (int k = 0; k < id_cell.size(); k++) {
						int id = id_cell[k];
						particleList[id].removeOccupiedCells(index(i,j));
					}
					id_cell.clear();
					id_cell.push_back(-1);
				}

			} // isBoundaryCell

		} // for height
	} // for width
	//std::cout << "baby good night" << std::endl;
	//std::cout << "pls sir" << std::endl;
	delete height_map;

	//std:: cout << "after delete height map" << std::endl;
	height_map = h_map;

//	check();

	//std::cout << "Finished erodeHeightBOi" << std::endl;
}

bool System::isResidualDropletCell(int i, int j) {
	// QUESTIONABLE NAMING CONVENTION
	// either the text is asking to skip residuals but not statics --> update condition
	// or we are skipping all statics (residuals included) --> rename method
//	std::cout << "hey babe it's residual" << std::endl;
	if (id_map[index(i,j)][0] == -1)
		return false;

	for (int k = 0; k < id_map[index(i,j)].size(); k++) {
		int id = id_map[index(i,j)][k];
		if (particleList[id].isStatic())
			return true;
	}

	return false;
}

bool System::isBoundaryCell(int i, int j) {
	if (height_map[index(i,j)] > 0.0f) {
		for (int m = -1; m <= 1; m++) {
			for (int n = -1; n <= 1; n++) {
				if (height_map[index(i+m, j+n)] == 0.0f)
					return true;
			}
		}
	}
	return false;
}

void System::deleteOutOfBoundDroplets() {
	std::cout << "DELETION" << std::endl;
	std::unordered_map<int, Particle> updatedList = particleList;
	for (auto i = particleList.begin(); i != particleList.end(); i++) {
		Particle &p = i->second;
		if (p.getListOfOccupiedCells().size() == 0) {
			std::cout << "  assassinate " << p.getID() << std::endl;
			updatedList.erase(p.getID());
		}
	}

	particleList = updatedList;
}

void System::mergeDroplets() {
/*...I...I might just brute force this...I don't know enough about collision detection...
 * hm...checking bottom up... you check 4 neighbors (TL, T, TR, R), ignoring the 4 "behind you" since you already checked those relations
 * ...but then how do you change all the old ids on the map to the new one?
 * 		.. have particles know about their grids? --> unordered_set of positions
 *
 * either check 4 directions, every cell (4 * SIZE) OR 8 directions, every cell with particle (at most 8 * SIZE)
 *  for the every particle one to be worth it, 4x > 8m, so 0.5 > m/x, particles must cover less than half the cells on the grid
 *
 *
 *  unordered_map<int, Particle*> copy = particles
 *  for every particle
 *  	for every cell the particle occupies
 *  		check 8 directions
 *  		if id != -1 and id != particle.id
 *  			create new particle (see below) and add to copy
 *  			replace all of the two particle's grid cell ids with the new particle's
 *  			delete those two particles from copy
 *  particles = copy
 *
 *
 *
 *
 * check id map
 * adjacent if there exist (x0,y0) and (x1, y1) such that
 * 		1. id at (x0,y0) != id at (x1,y1)
 * 		2. |x0-x1| <= 1 && |y0-y1| <= 1
 * 	if adjacent,
 * 		create new particle
 * 			velocity = mu * (m0v0 + m1v1)/(m0+m1)	--> mu > 1
 * 			position = water drop with lowest position
 * 			mass = sum of masses
 */

	std::cout << "MERGE TIME SON" << std::endl;

	std::unordered_map<int, Particle> updatedList = particleList;

	for (auto pID = particleList.begin(); pID != particleList.end(); pID++) {
		Particle &particle = pID->second;
		int particleID = particle.getID();
//		std::cout << "checking with " << particleID << std::endl;
		int neighborID = neighboringDroplet(particle);
		if (neighborID == -1) {
//			std::cout << "no merge" << std::endl;
			continue;
		}
//		std::cout << "merging neighbor " << neighborID << std::endl;
		mergeParticles(updatedList, particleID, neighborID);
//		std::cout << "------------------------" << std::endl;
	}

	particleList = updatedList;
}

int System::neighboringDroplet(Particle &particle) {
	int particleID = particle.getID();

	// go through particle's list of occupied cells
	std::unordered_set<int> occupiedCells = particle.getListOfOccupiedCells();
	for (auto cellIndex = occupiedCells.begin(); cellIndex != occupiedCells.end(); cellIndex++) {
		glm::ivec2 cell = gridPosition(*cellIndex);

		// check every neighboring cell
		for (int m = -1; m <= 1; m++) {
			for (int n = -1; n <= 1; n++) {
				std::vector<int> &neighbor_id_cell = id_map[index(cell.x + m, cell.y + n)];

				// check every neighboring cell's list of ids
				for (int k = 0; k < neighbor_id_cell.size(); k++) {
					int neighborID = neighbor_id_cell[k];
					if (neighborID != -1 && neighborID != particleID) {
						// check if one is the residual of the other
						// should we be using updatedList instead??
						if ( particleList[neighborID].isResidualOf(particleID) ||
								particle.isResidualOf(neighborID) )
							continue;
						else
							return neighborID;
					}

				}
			}
		}

	}
	return -1;
}

void System::mergeParticles(std::unordered_map<int, Particle> &list, int particleID, int neighborID) {
	std::cout << "  merge particles " << particleID << " + " << neighborID;
	Particle &particle = list[particleID];
	Particle &neighbor = list[neighborID];

	// new particle position is the lowest of the two particleList' positions
	glm::vec2 np_position;
	if (particle.getPosition().y < neighbor.getPosition().y)
		np_position = particle.getPosition();
	else
		np_position = neighbor.getPosition();
//	std::cout << "footloose" << std::endl;

	// new particle mass = sum of masses
	float np_mass = particle.getMass() + neighbor.getMass();

	// velocity = mu * (m0v0 + m1v1)/(m0+m1)	--> mu > 1
	float particleMass = particle.getMass();
	float neighborMass = neighbor.getMass();
	glm::vec2 np_velocity = MU * ( particleMass*particle.getVelocity() +
			neighborMass*neighbor.getVelocity() ) / (particleMass + neighborMass);
//	std::cout << "chub chubs" << std::endl;


	int np_id;
	int old_id;
	// 	CHOOSE NP_ID WITH
	// IF STATIC + STATIC --> THE ONE WITH MORE MASS
	// IF MOVING + MOVING --> THE LOWER ONE
	// IF STATIC + MOVING --> THE MOVING
	if (particle.isStatic() && neighbor.isStatic()){
		np_id = (particle.getMass() >= neighbor.getMass()) ? particleID : neighborID;
	}
	else if (!particle.isStatic() && !neighbor.isStatic())
		np_id = (np_position == particle.getPosition()) ? particleID : neighborID;
	else
		np_id = (!particle.isStatic()) ? particleID : neighborID;

	old_id = (np_id == particleID) ? neighborID : particleID;



	// modify particle for new (merged) particle
//	int np_id = particleID;
//	int old_id = neighborID;

//	if ( (neighbor.getListOfOccupiedCells()).size() > (particle.getListOfOccupiedCells()).size() ) {
//	if ( neighbor.getTimeSinceLastResidual() > particle.getTimeSinceLastResidual() ) {
//		np_id = neighborID;
//		old_id = particleID;
//	}
	std::unordered_set<int> mergeCells = list[old_id].getListOfOccupiedCells();

//	std::cout << "high neighbor" << std::endl;
	list[np_id].setPosition(np_position);
	list[np_id].setMass(np_mass);
	list[np_id].setVelocity(np_velocity);
	list[np_id].addOccupiedCells(mergeCells);
	list[np_id].resetTimeSinceLastResidual();
	list[np_id].clearParent();
//	std::cout << "brainwash complete" << std::endl;

	// modify ID map; merge/change the two particles' ids
//	for (auto mc = mergeCells.begin(); mc != mergeCells.end(); mc++) {
//		auto id = std::find(id_map[*mc].begin(), id_map[*mc].end(), np_id);
//		if (*id == np_id)
//			id_map[*mc].erase(id);
//
//		std::replace(id_map[*mc].begin(), id_map[*mc].end(), old_id, np_id);
//
//	}

	// alternative / more intuitive way for modifying ID map
	for (auto mc = mergeCells.begin(); mc != mergeCells.end(); mc++) {
		auto id = std::find(id_map[*mc].begin(), id_map[*mc].end(), old_id);
		if (*id == old_id)
			id_map[*mc].erase(id);
		if (std::count(id_map[*mc].begin(), id_map[*mc].end(), np_id) == 0)
			id_map[*mc].push_back(np_id);
	}
	std::cout << " --> " << np_id << std::endl;

	std::cout << "  immigration complete" << std::endl;

	// delete old particle in new list
	// and indicate new id on old list
	list.erase(old_id);
	particleList[old_id].mergeID(np_id);
	std::cout << "  death of " << old_id << std::endl;
//	std::cout << "assassination successful ; " << old_id << " now " << particleList[old_id].getID() << std::endl;
}





// for now, do wrap-around
int System::index(glm::vec2 pos) {
	// reality to grid
	glm::ivec2 p = pos / GRID_LENGTH;

	if (p.x < 0)
		p.x = (p.x % MAP_WIDTH) + MAP_WIDTH;
	if (p.x >= MAP_WIDTH)
		p.x = p.x % MAP_WIDTH;

	if (p.y < 0)
		p.y = (p.y % MAP_HEIGHT) + MAP_HEIGHT;
	if (p.y >= MAP_HEIGHT)
		p.y = p.y % MAP_HEIGHT;

	if (p.y*MAP_WIDTH + p.x > MAP_SIZE-1)
		std::cout << "ya weirdo " << p.x << ", " << p.y << std::endl;

	//std::cout << "anywho " << p.x << ", " << p.y << " = " << (p.y*MAP_WIDTH + p.x) << " ??? " << MAP_SIZE-1 << " " << MAP_WIDTH << " " << MAP_HEIGHT << std::endl;

	return p.y*MAP_WIDTH + p.x;
}

int System::index(int x, int y) {
	if (x < 0)
		x = (x % MAP_WIDTH) + MAP_WIDTH;
	if (x >= MAP_WIDTH)
		x = x % MAP_WIDTH;

	if (y < 0)
		y = (y % MAP_HEIGHT) + MAP_HEIGHT;
	if (y >= MAP_HEIGHT)
		y = y % MAP_HEIGHT;

	//std::cout << "anywho " << x << ", " << y << " = " << (y*MAP_WIDTH + x) << " ??? " << MAP_SIZE-1 << " " << MAP_WIDTH << " " << MAP_HEIGHT << std::endl;

	if ((y*MAP_WIDTH + x) > MAP_SIZE-1)
			std::cout << "ya weirdo " << x << ", " << y << " = " << (y*MAP_WIDTH + x) << " ??? " << MAP_SIZE-1 << " " << MAP_WIDTH << " " << MAP_HEIGHT << std::endl;

	return y*MAP_WIDTH + x;
}

glm::vec2 System::position(int index) {
	int x = index % MAP_WIDTH;
	int y = index / MAP_WIDTH;

	float grid_x = ((float)x + 0.5f) * GRID_LENGTH;
	float grid_y = ((float)y + 0.5f) * GRID_LENGTH;

	return glm::vec2(grid_x, grid_y);
}

glm::vec2 System::position(int x, int y) {
	float grid_x = ((float)x + 0.5f) * GRID_LENGTH;
	float grid_y = ((float)y + 0.5f) * GRID_LENGTH;

	return glm::vec2(grid_x, grid_y);
}

glm::ivec2 System::gridPosition(int index) {
	int i = index % MAP_WIDTH;
	int j = index / MAP_WIDTH;

	return glm::ivec2(i,j);
}

glm::ivec2 System::gridPosition(glm::vec2 pos) {
	return glm::ivec2(pos / GRID_LENGTH);
}



