#pragma once

#include <vector>
#include <map>
#include <string>
#include <random>
#include <chrono>  // for the high-resolution clock


#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>

#include "boid.h"
#include "geometry.h"
#include "../aux/profiler.h"

using Value = std::pair<Box, Boid*>;  

class World {
private:
    std::vector<Boid> boids;
    boost::geometry::index::rtree<Value, boost::geometry::index::quadratic<64>> rtree;
    // todo: optimize (or parametrize) rtree quadratic<64>
    int width;
    int height;
    int margin;

public:

    struct Config {
        int width;
        int height;
        int margin;
    };
    Config config_;

    World(const std::map<std::string, std::string>& configMap);  // Constructor to initialize from JSON
    
    World(Config &config);
    std::vector<Boid> getBoids();
    void update();

    void handleMargins(Boid& boid);
    std::vector<Boid> getNeighbors(Boid& boid, float radius) const;
    void popualateRtree();

};
