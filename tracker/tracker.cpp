
#include "tracker.h"

Tracker::Tracker(const std::string& filename, const World& world, int interval) 
    : filename_(filename), interval_(interval), stepCounter_(0) {
    // Initialize file and write headers
    std::ofstream outFile(filename_);
    outFile << "timepoint";
    for (size_t i = 0; i < world.getBoids().size(); ++i) {
        outFile << ",boid" << i << "_x,boid" << i << "_y";
    }
    outFile << std::endl;
}

void Tracker::update(const World& world) {
    // Only log every 'interval_' steps
    if (++stepCounter_ % interval_ != 0) return;

    // Get boids from the world and write their positions to the file
    std::ofstream outFile(filename_, std::ios_base::app); // Open in append mode
    outFile << stepCounter_; // Assuming stepCounter_ aligns with timepoints
    const auto& boids = world.getBoids();
    for (const auto& boid : boids) {
        outFile << "," << boid.getPosition().x() << "," << boid.getPosition().y();
    }
    outFile << std::endl;
}


