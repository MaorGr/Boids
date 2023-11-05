#pragma once

#include <vector>
#include <map>
#include <string>
#include <random>

#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>

#include "boid.h"
#include "geometry.h"
// #include "../external/quad/Quadtree.h"




// Use full namespace here to make it clear to users of this header what's going on.
// using Point = boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian>;
// using Box = boost::geometry::model::box<Point>;
using Value = std::pair<Box, Boid*>;  // Assuming each Boid has a unique unsigned integer ID

class World {
private:
    std::vector<Boid> boids;
    boost::geometry::index::rtree<Value, boost::geometry::index::quadratic<10>> rtree;
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
