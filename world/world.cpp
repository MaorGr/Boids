#include "world.h"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

int Boid::maxId = 0;



World::World(World::Config &config) : config_(config) {


    this->width = config.width;
    this->height = config.height;
    this->margin = config.margin;

    float max_speed = 100;
    std::random_device rd;  // Used to initialize the seed
    std::mt19937 gen(rd()); // Mersenne Twister pseudo-random generator
    std::uniform_real_distribution<> dis(0.0, 1.0); // Uniform distribution between 0.0 and 1.0

    for (int i = 0; i < 1000; i++) {
        float dir = dis(gen) * 2.0f * M_PI;
        float v0 = dis(gen) * max_speed;
        float vx = std::sin(dir) * v0;
        float vy = std::cos(dir) * v0;

        
        boids.emplace_back(margin + int(dis(gen) * width * 10.0f) % (width - 2* margin), 
                           margin + int(dis(gen) * height * 10.0f) % (height - 2* margin),
                           vx, vy);

    }

    popualateRtree();


}

void World::popualateRtree() {
    for (auto& boid : boids) {
        Value value = Value(boid.getBox(), &boid);
        rtree.insert(value);
    }
}

std::vector<Boid> World::getBoids() {
    return boids;
}

void World::update() {

    std::chrono::duration<double> duration_getNeighbors_flock = std::chrono::duration<double>::zero();
    std::chrono::duration<double> duration_doFlocking = std::chrono::duration<double>::zero();
    std::chrono::duration<double> duration_getNeighbors_avoid = std::chrono::duration<double>::zero();
    std::chrono::duration<double> duration_doAvoid = std::chrono::duration<double>::zero();
    std::chrono::duration<double> duration_rtree_update = std::chrono::duration<double>::zero();

    for (auto& boid : boids) {
        handleMargins(boid); 
        decltype(getNeighbors(boid, 100)) ngh_flock;
        decltype(getNeighbors(boid, 100)) ngh_avoid;
        {
            Profiler profile(duration_getNeighbors_flock);
            ngh_flock = getNeighbors(boid, 40);
        }
        {
            Profiler profile(duration_doFlocking);
            boid.doFlocking(ngh_flock);
        }
        {
            Profiler profile(duration_getNeighbors_avoid);
            ngh_flock = getNeighbors(boid, 8);
        }
        {
            Profiler profile(duration_doAvoid);
            boid.updateAvoidanceDirection(ngh_avoid);
        }
        boid.update();
    }
    {
        Profiler profile(duration_rtree_update);
        rtree.clear();
        this->popualateRtree();
    }

    std::cout << "NF: " << duration_getNeighbors_flock.count()
        << " DF: " << duration_doFlocking.count() 
        << " NA: " << duration_getNeighbors_avoid.count() 
        << " DA: " << duration_doAvoid.count() 
        << " RT: " << duration_rtree_update.count() 
        << std::endl;
}



void World::handleMargins(Boid& boid) {

    Eigen::Vector2f position = boid.getPosition();
    Eigen::Vector2f velocity = boid.getVelocity();
    if (position.x() < margin) {
        position[0] = 2 * margin - position.x();
        velocity[0] = velocity.x() * -1.0f;
    } 
    else if (position.x() > width - margin) {
        position[0] = 2 * (width - margin) - position.x();
        velocity[0] = velocity.x() * -1.0f;
    } 
    if (position.y() < margin) {
        position[1] = 2 * margin - position.y();
        velocity[1] = velocity.y() * -1.0f;
    } 
    else if (position.y() > height - margin) {
        position[1] = 2 * (height - margin) -  position.y();
        velocity[1] = velocity.y() * -1.0f;
    } 
    boid.setPosition(position);
    boid.setVelocity(velocity);
}

std::vector<Boid> World::getNeighbors(Boid& boid, float radius) const {

    Box bbox = boid.getBox();
    Box qbox = Box(Point(boid.getPosition().x() - radius, boid.getPosition().y() - radius),
                   Point(boid.getPosition().x() + radius, boid.getPosition().y() + radius));
    // std::cout << "[" << qbox.left << ", " << qbox.left + qbox.width << "], [" << qbox.top << ", " << qbox.top + qbox.height << "]" << std::endl; 
    std::vector<Value> result;
    rtree.query(bgi::intersects(qbox), std::back_inserter(result));

    // std::cout << " > " << neighbors.size() << std::endl;
    std::vector<Boid> ngh = std::vector<Boid>();
    for (auto candidate : result) {
        Vector2f delta = boid.getPosition() - candidate.second->getPosition(); 
        float dist = delta.norm();
        if (dist < radius) {
            ngh.push_back(*(candidate.second));
        }
    }
    return ngh;
}