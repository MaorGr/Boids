#include "world.h"
#include <thread>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

int Boid::maxId = 0;

World::World(World::WorldConfig &config) : config_(config) {

    this->width = config.width;
    this->height = config.height;
    this->margin = config.margin;
    this->dt = config.dt;
    this->boid_count = config.boid_count;
    this->turn_factor = config.turn_factor;
    LOG(INFO) << "??" << config_.potential_path << std::endl;
    if (!config_.potential_path.empty()) {
        LOG(INFO) << "loading potential from" << config_.potential_path << std::endl;
        this->potential = cv::imread(config_.potential_path, cv::IMREAD_GRAYSCALE);
        if (this->potential.empty()) {
            LOG(INFO) << "Error: Image could not be loaded from the path." << std::endl;
        }
        this->calculateForce();
    }
}

void World::calculateForce() {

    // todo: add and parametrize a gaussiaon filter
    cv::Sobel(this->potential, this->force_x, CV_32F, 1, 0);
    cv::Sobel(this->potential, this->force_y, CV_32F, 0, 1);
}

void World::populate(Boid::BoidConfig &boid_config) {

    LOG(INFO) <<  ">>" << "populating: " << this->boid_count <<  std::endl;


    std::random_device rd;  // Used to initialize the seed
    std::mt19937 gen(rd()); // Mersenne Twister pseudo-random generator
    std::uniform_real_distribution<> dis(0.1, 1.0); // Uniform distribution between 0.0 and 1.0

    for (int i = 0; i < this->boid_count; i++) {
        float dir = dis(gen) * 2.0f * M_PI;
        float v0 = dis(gen) * (boid_config.max_speed - boid_config.min_speed) + boid_config.min_speed;
        float vx = std::sin(dir) * v0;
        float vy = std::cos(dir) * v0;
        
        this->boids.emplace_back(margin + fmod(float(dis(gen) * width * 10.0f), float(width - 2.0 * margin)), 
                           margin + fmod(float(dis(gen) * height * 10.0f), float(height - 2.0 * margin)),
                           vx, vy, boid_config, i);

        

    }

    popualateRtree();


}

void World::popualateRtree() {
    for (auto& boid : boids) {
        Value value = Value(boid.getBox(), &boid);
        Box box = value.first;
        const Point& min_corner = box.min_corner();
        const Point& max_corner = box.max_corner();
        rtree.insert(value);
    }
}

std::vector<Boid> World::getBoids() const {
    return boids;
}

void World::update() {

    std::chrono::duration<double> duration_getNeighbors_flock = std::chrono::duration<double>::zero();
    std::chrono::duration<double> duration_doFlocking = std::chrono::duration<double>::zero();
    std::chrono::duration<double> duration_getNeighbors_avoid = std::chrono::duration<double>::zero();
    std::chrono::duration<double> duration_doAvoid = std::chrono::duration<double>::zero();
    std::chrono::duration<double> duration_rtree_update = std::chrono::duration<double>::zero();

    for (auto& boid : boids) {
        // handleMargins(boid);
        handleForce(boid);
    }

    unsigned num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads(num_threads);
    auto boid_per_thread = boids.size() / num_threads;



    for (unsigned i = 0; i < num_threads; ++i) {
        threads[i] = std::thread([&, i]() {

            auto start = boids.begin() + i * boid_per_thread;
            auto end = (i == num_threads - 1) ? boids.end() : start + boid_per_thread;
            for (auto it = start; it != end; ++it) {
                
                decltype(getNeighbors(*it, 1)) ngh_flock;
                decltype(getNeighbors(*it, 1)) ngh_avoid;
                {
                    Profiler profile(duration_getNeighbors_flock);
                    ngh_flock = getNeighbors(*it, it->getSenseRadius());
                }
                {
                    Profiler profile(duration_doFlocking);
                    it->doFlocking(ngh_flock);
                }
                {
                    Profiler profile(duration_getNeighbors_avoid);
                    ngh_flock = getNeighbors(*it, it->getAvoidRadius());
                }
                {
                    Profiler profile(duration_doAvoid);
                    it->updateAvoidanceDirection(ngh_avoid);
                }
            }
            LOG(INFO) << "thread" << i 
                << " NF: " << duration_getNeighbors_flock.count()
                << " DF: " << duration_doFlocking.count() 
                << " NA: " << duration_getNeighbors_avoid.count() 
                << " DA: " << duration_doAvoid.count() 
                << std::endl;
        });
    }
    for (auto& t : threads) {
       t.join();
    }
    LOG(INFO) << this->dt << std::endl;


    std::random_device rd;
    std::mt19937 gen(rd());
    
    // TODO(maor): parametrize
    double mean = 0.0;  // No bias in noise
    double stddev = 0.1;
    std::normal_distribution<double> d(mean, stddev);

    for (auto& boid : boids) {
        Eigen::Vector2f noise = Eigen::Vector2f(d(gen), d(gen));
        boid.setVelocity(boid.getVelocity() += noise);
    }

    for (auto& boid : boids) {
        boid.update(this->dt);
    }
    {
        Profiler profile(duration_rtree_update);
        rtree.clear();
        this->popualateRtree();
    }
    LOG(INFO) << " RT: " << duration_rtree_update.count() << std::endl;
}

void World::handleForce(Boid &boid) {
    Eigen::Vector2f position = boid.getPosition();
    Eigen::Vector2f velocity = boid.getVelocity();
    if (position.x() < 0) {
        position[0] = -1.0f * position.x();
        velocity[0] = velocity.x() * -1.0f;
    } 
    if (position.x() > width) {
        position[0] = 2 * (width) - position.x();
        velocity[0] = velocity.x() * -1.0f;
    } 
    if (position.y() < 0) {
        position[1] = -1.0f - position.y();
        velocity[1] = velocity.y() * -1.0f;
    } 
    if (position.y() > height) {
        position[1] = 2 * (height) -  position.y();
        velocity[1] = velocity.y() * -1.0f;
    } 
    
    float forceX = this->force_x.at<float>(int(position[0]), int(position[1]));
    float forceY = this->force_y.at<float>(int(position[0]), int(position[1]));
    velocity[0] += forceX * this->turn_factor;
    velocity[1] += forceY * this->turn_factor;

    boid.setPosition(position);
    boid.setVelocity(velocity);

}

void World::handleMargins(Boid& boid) {

    Eigen::Vector2f position = boid.getPosition();
    Eigen::Vector2f velocity = boid.getVelocity();

    if (position.x() < margin) {
        velocity[0] += this->turn_factor;
    } 
    if (position.x() < 0) {
        position[0] = -1.0f * position.x();
        velocity[0] = velocity.x() * -1.0f;
    } 
    if (position.x() > width - margin) {
        velocity[0] -= this->turn_factor;
    } 
    if (position.x() > width) {
        position[0] = 2 * (width) - position.x();
        velocity[0] = velocity.x() * -1.0f;
    } 
    if (position.y() < margin) {
        velocity[1] += this->turn_factor;
    } 
    if (position.y() < 0) {
        position[1] = -1.0f - position.y();
        velocity[1] = velocity.y() * -1.0f;
    } 
    if (position.y() > height - margin) {
        velocity[1] -= this->turn_factor;
    } 
    if (position.y() > height) {
        position[1] = 2 * (height) -  position.y();
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