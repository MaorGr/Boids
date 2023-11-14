#include "boid.h"
#include <SFML/Graphics.hpp> 
#include <iostream> // todo(maor): replace with a logging system


Boid::Boid(float x, float y, float vx, float vy, const Boid::BoidConfig& config, unsigned int id)
        : position(x, y), velocity(vx, vy), config_(config) {
        // Initialize other Boid members using config_
        this->max_speed = config_.max_speed;
        this->min_speed = config_.min_speed;
        this->avoid_radius = config_.avoid_radius;
        this->sense_radius = config_.sense_radius;
        this->avoid_factor = config_.avoid_factor;
        this->matching_factor = config_.matching_factor;
        this->centering_factor = config_.matching_factor;
        this->id = id;

        // this->position[0] = x;
        // this->position[1] = y;
        // this->velocity[0] = vx;
        // this->velocity[1] = vy;
}

// Boid::Boid(float x, float y, float vx, float vy) : Boid(x, y) {
//     this->velocity = Vector2f(vx, vy);
// }

// Boid::Boid(float x, float y) : position(x, y), velocity(0, 0), acceleration(0, 0) {
//     // this->position = Point(x, y);
//     // this->velocity = Point(0, 0);
//     // this->acceleration = Point(0, 0);
//     touchRadius = 1;

//     // this->bbox = Box(Point(x - touchRadius, y - touchRadius), Point(x + touchRadius, y + touchRadius));
//     this->id = this->maxId;
//     this->maxId += 1;
// }

Vector2f Boid::getPosition() const {
    return this->position;
}

void Boid::setPosition(Vector2f position) {
    this->position = position;
}

Vector2f Boid::getVelocity() const {
    return this->velocity;
}

void Boid::setVelocity(Vector2f velocity){
    this->velocity = velocity;
}

float Boid::getSenseRadius() const {
    return this->sense_radius;
}

void Boid::setSenseRadius(float radius){
    this->sense_radius = radius;
}

float Boid::getAvoidRadius() const {
    return this->avoid_radius;
}

void Boid::setAvoidRadius(float radius){
    this->avoid_radius = radius;
}

unsigned int Boid::getId() { 
    return id; 
}


Box Boid::getBox() {
    // todo(maor): invalidate?
    int touchRadius = 1;
    this->bbox = Box(Point(this->position.x() - touchRadius, this->position.y() - touchRadius), Point(this->position.x() + touchRadius, this->position.y() + touchRadius));
    return this->bbox;
}

void Boid::updateAvoidanceDirection(const std::vector<Boid>& neighbors) {
    Vector2f repulsion = Vector2f(0, 0); 
    // Vector2f predictiveAvoidance = Vector2f(0, 0);

    for (Boid neighbor : neighbors) {
        // Calculate distance to neighbor
        Vector2f delta = this->getPosition() - neighbor.getPosition();
        // float distance = delta.norm();
        repulsion -= delta;
    }
    this->velocity += this->velocity + avoid_factor * repulsion;

    
 

        // float PREDICTIVE_LOOKAHEAD = 0.1;
        // float COLLISION_THRESHOLD = 0.1;
        // // Predictive Avoidance
        // Eigen::Vector2f futureBoidPosition = this->position + this->velocity * PREDICTIVE_LOOKAHEAD;
        // Eigen::Vector2f futureNeighborPosition = neighbor.position + neighbor.velocity * PREDICTIVE_LOOKAHEAD;
        // Eigen::Vector2f delta_future =futureBoidPosition - futureNeighborPosition;
        // if (delta_future.norm() < COLLISION_THRESHOLD) {
        //     Eigen::Vector2f avoidanceDir = futureBoidPosition - futureNeighborPosition;
        //     avoidanceDir.normalize();
        //     predictiveAvoidance += avoidanceDir;
        // }
    // }

    // Combine forces and normalize
    // Eigen::Vector2f combined = repulsionForce + predictiveAvoidance;
    // if (combined.length() > 0) {
    //     combined.normalize();
    // std::cout << "old: " << this->velocity.x << ", " << this->velocity.y;
    // this->velocity = this->velocity + repulsion_factor * combined;
    // std::cout << "--> new: " << this->velocity.x << ", " << this->velocity.y << std::endl;
    // this->velocity += 0.000001f * combined;
    // }
}


void Boid::doFlocking(std::vector<Boid> &neighbors) {
    auto result = this->getBoidsAvgInfo(neighbors);
    
    if (result) {
        Eigen::Vector2f avgPos = result->first;
        Eigen::Vector2f avgVel = result->second;
        // Use avgPos and avgVel as needed

        // Eigen::Vector2f deltaAcc = factorP * deltaPos + factorV * deltaVel;
        // this->acceleration = this->acceleration + deltaAcc;
        this->velocity += (avgVel) * this->matching_factor;
        this->velocity += (avgPos) * this->centering_factor;
    }
}

void Boid::update(float dt) {

    // this->velocity = this->velocity + this->acceleration * dt;
    if (this->velocity.norm() > this->max_speed) {
        this->velocity = this->velocity / (this->velocity.norm() / this->max_speed); 
    } else if (this->velocity.norm() < this->min_speed){
        this->velocity = this->velocity / (this->velocity.norm() / this->min_speed);
    }
    this->position = this->position + this->velocity * dt;
}

std::optional<std::pair<Vector2f, Vector2f>> Boid::getBoidsAvgInfo(const std::vector<Boid>& boids) {
    Vector2f sumPos(0, 0);
    Vector2f sumVel(0, 0);
    int count = 0;
    for (const auto& other : boids) {
        Vector2f deltaP = other.position - this->position;
        Vector2f deltaV = other.velocity - this->velocity;
        // todo(maor): explore weight by inverse distance
        // const float dist = deltaP.norm();
        // float velW = fmax(0, (perceptionRadius - dist) / perceptionRadius);
        // float posW = fmax(0, (perceptionRadius - dist) / perceptionRadius);
        sumPos += deltaP;
        sumVel += deltaV;        
        count++;

    }
    if (count > 0) {
        return std::make_pair(
            (sumPos / float(count)), 
            (sumVel / float(count))
        );
    } else {
        return {};  // No neighbors found
    }
}

