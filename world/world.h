#pragma once

#include <vector>
#include <map>
#include <string>
#include <random>
#include <chrono>  // for the high-resolution clock
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <glog/logging.h>
#include <opencv2/opencv.hpp>


#include "boid.h"
#include "geometry.h"
#include "../aux/profiler.h"
#include "../aux/config.h"

using Value = std::pair<Box, Boid*>;  

class World {
private:
    std::vector<Boid> boids;
    boost::geometry::index::rtree<Value, boost::geometry::index::quadratic<64>> rtree;
    // todo: optimize (or parametrize) rtree quadratic<64>
    int width;
    int height;
    float margin;
    float dt;
    int boid_count;
    float turn_factor;
    cv::Mat potential;
    cv::Mat force_x;
    cv::Mat force_y;

public:

    struct WorldConfig {
        int width;
        int height;
        float margin;
        float dt;
        int boid_count;
        float turn_factor;
        std::string potential_path;
    };
    WorldConfig config_;
    
    World(WorldConfig &config);
    std::vector<Boid> getBoids() const;
    void update();

    int getWidth() { return this->width; };
    int getHeight() { return this->height; };

    void keepInFrame(Boid& boid);
    void handleMargins(Boid& boid);
    void handleForce(Boid &boid);
    std::vector<Boid> getNeighbors(Boid& boid, float radius) const;
    void popualateRtree();
    void populate(Boid::BoidConfig &boid_config);


    void calculateForce();

};
