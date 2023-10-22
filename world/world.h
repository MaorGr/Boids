#pragma once

#include <vector>
#include "Boid.h"
#include "../external/quad/Quadtree.h"
#include <string>

class World {
private:
    std::vector<Boid> boids;

    template <typename GetBoxFunc>
    quadtree::Quadtree<Boid*, GetBoxFunc>& quadtree*> quadtree;  // Assuming this is your Quadtree definition

public:
    World(const std::string& configPath);  // Constructor to initialize from JSON
    void update();
    // Other necessary functions/methods
};