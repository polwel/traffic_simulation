//
// Created by Carlo on 18.10.2015.
//

#include <road.h>
#include <fstream>
#include <iomanip>
#include <algorithm>

#include <simulation.h>
#include <iostream>

extern Simulation * active_simulation;

#define DEFAULT_SPEEDLIMIT (s->getSettings()->v_desired)
#define DEFAULT_TIMEHEADWAY (s->getSettings()->t_desired)
#define DEFAULT_SPACEHEADWAY (s->getSettings()->d_desired)


Road::Road(Simulation const * const s)
    : Road(DEFAULT_SPEEDLIMIT, DEFAULT_TIMEHEADWAY, DEFAULT_SPACEHEADWAY, s)
{
}


Road::Road(double speed_limit, double time_headway, double space_headway, Simulation const * const s)
    : speed_limit(speed_limit), time_headway(time_headway), space_headway(space_headway), simulation(s), throughput(0)
{
}

Road::Road(const Road & other)
    : Road(other, other.simulation)
{
}

Road::Road(const Road & other, Simulation const * const s)
    : speed_limit(other.speed_limit), time_headway(other.time_headway), space_headway(other.space_headway), simulation(s), throughput(0)
{
    cars.resize(other.cars.size());
    for (unsigned int i = 0; i < other.cars.size(); ++i) {
        cars[i] = new Car(*other.cars[i], this);
    }
    reIndex();
}

Road::Road(const std::vector<double> &pos, Simulation const * const s)
    : speed_limit(DEFAULT_SPEEDLIMIT), time_headway(DEFAULT_TIMEHEADWAY), space_headway(DEFAULT_SPACEHEADWAY), simulation(s)
{
    populate(pos);
}

Road::~Road()
{
    for (unsigned int i = 0; i < cars.size(); ++i) {
        delete cars[i];
    }
}

void Road::reIndex() const
{
    for (unsigned int i = 1; i < cars.size(); ++i) {
        cars[i]->prev_car = cars[i - 1];
        cars[i - 1]->next_car = cars[i];
    }
    cars[0]->prev_car = cars[cars.size()-1];
    cars[cars.size() - 1]->next_car = cars[0];
}


void Road::populate(const std::vector<Car::position> & pos)
{
	cars.clear();
	cars.resize(pos.size());
	for (unsigned int i = 0; i < pos.size(); ++i)
		cars[i] = new Car(pos[i], this);

    reIndex();        
}

void Road::populate(unsigned int n)
{
    cars.clear(); //TODO: MEMORY LEAK!!!!!!
    cars.resize(n);
    for (unsigned int i = 0; i < n; ++i) {
        cars[i] = new Car(length/n * i, this);
    }
    reIndex();
}


void Road::writePositions(std::string filename) const
{
    std::ofstream out(filename.c_str());

    out << std::setprecision(5);
	for (unsigned int i = 0; i < cars.size(); ++i)
    {
		out << cars[i]->toString() << " \t ";
    }
    out << std::endl;

    for (unsigned int i = 0; i < cars.size(); ++i)
    {
        out << cars[i]->getVelocity() << " \t ";
    }

    out << std::endl;
    out.close();
}


std::vector<double> Road::getPositions() const
{
    std::vector<double> result;
    result.resize(cars.size());
    for (unsigned int i = 0; i < result.size(); ++i)
        result[i] = cars[i]->getPosition();
    return result;
}

std::vector<double> Road::getVelocities() const
{
    std::vector<double> result;
    result.resize(cars.size());
    for (unsigned int i = 0;  i < result.size(); ++i)
        result[i] = cars[i]->getVelocity();
    return result;
}

unsigned int Road::getTroughput() const
{
    return this->throughput;
}

void Road::resetTroughput()
{
    this->throughput = 0;
}

void Road::incThroughput()
{
    this->throughput++;
}

double Road::getLength() const
{
	return length;
}

void Road::addObstacle(const Obstacle & o)
{
	obstacles.push_back(o);
}

void Road::addTrafficLight(const TrafficLight & l)
{
    traffic_lights.push_back(l);
}


void Road::clearObstacles()
{
	obstacles.clear();
}

/**
* Returns the desired speed limit at position x. It is given by the most stringent (i.e. lowest) limit of any obstacle at that position.
*/
double Road::speedLimit(double x) const
{
    double result = speed_limit;

    for (unsigned i = 0; i < obstacles.size(); ++i) {
        if (obstacles[i].start < x && x < obstacles[i].end)
            result = std::min(result, obstacles[i].speed_limit);
    }

    return result;
}

/**
* Returns the desired time headway at position x. It is given by the most stringent (i.e. highest) limit of any obstacle at that position.
*/
double Road::timeHeadway(double x) const
{
    double result = time_headway;

    for (unsigned i = 0; i < obstacles.size(); ++i) {
        if (obstacles[i].start < x && x < obstacles[i].end)
            result = std::max(result, obstacles[i].time_headway);
    }

    return result;
}

/**
* Returns the desired space headway at position x. It is given by the most stringent (i.e. highest) limit of any obstacle at that position.
*/
double Road::spaceHeadway(double x) const
{
    double result = time_headway;

    for (unsigned i = 0; i < obstacles.size(); ++i) {
        if (obstacles[i].start < x && x < obstacles[i].end)
            result = std::max(result, obstacles[i].space_headway);
    }

    return result;
}

int Road::find(const Car * c) const
{
    for (unsigned int i = 0; i < cars.size(); ++i)
        if (cars[i] == c)
            return i;

    return -1;
}

void Road::update(const double dt)
{
    for(unsigned int id = 0; id < cars.size(); ++ id)
    {
        cars[id]->update_postion(dt);
    }
}