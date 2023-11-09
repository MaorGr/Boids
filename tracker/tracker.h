#pragma once

#include <fstream>
#include <vector>
#include <string>
#include "../world/world.h"
#include "../world/boid.h"

class Tracker {
public:
    Tracker(const std::string& filename, const World& world, int interval);
    void update(const World& world);

private:
    std::string filename_;
    int interval_; // Log the positions every 'interval_' steps
    int stepCounter_; // Keep track of the number of steps
};
