#include "boid.h"
#include <SFML/Graphics.hpp> 
#include <iostream> // todo(maor): replace with a logging system

const float DT = 1.0f; //TODO(remove)
const float MAX_SPEED = 6.0f;
const float MIN_SPEED = 3.0f;


Boid::Boid(float x, float y, float vx, float vy) : Boid(x, y) {
    this->velocity = Vector2f(vx, vy);
}

Boid::Boid(float x, float y) : position(x, y), velocity(0, 0), acceleration(0, 0) {
    // this->position = Point(x, y);
    // this->velocity = Point(0, 0);
    // this->acceleration = Point(0, 0);
    touchRadius = 2.5;
    sensingRadius = 50;
    // this->bbox = Box(Point(x - touchRadius, y - touchRadius), Point(x + touchRadius, y + touchRadius));
    this->id = this->maxId;
    this->maxId += 1;
}

Vector2f Boid::getPosition() {
    return this->position;
}

void Boid::setPosition(Vector2f position) {
    this->position = position;
}

Vector2f Boid::getVelocity() {
    return this->velocity;
}

void Boid::setVelocity(Vector2f velocity){
    this->velocity = velocity;
}

Box Boid::getBox() {
    // todo(maor): invalidate?
    this->bbox = Box(Point(this->position.x() - touchRadius, this->position.y() - touchRadius), Point(this->position.x() + touchRadius, this->position.y() + touchRadius));
    return this->bbox;
}

void Boid::updateAvoidanceDirection(const std::vector<Boid>& neighbors) {
    Vector2f repulsion = Vector2f(0, 0); 
    // Vector2f predictiveAvoidance = Vector2f(0, 0);

    float repulsion_factor = 0.15f;

    for (Boid neighbor : neighbors) {
        // Calculate distance to neighbor
        Vector2f delta = this->getPosition() - neighbor.getPosition();
        // float distance = delta.norm();
        repulsion -= delta;
    }
    this->velocity += this->velocity + repulsion_factor * repulsion;

    
 

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

    float matchingFactor = 0.05;
    float centeringFactor = 0.0005;

    
    if (result) {
        Eigen::Vector2f avgPos = result->first;
        Eigen::Vector2f avgVel = result->second;
        // Use avgPos and avgVel as needed

        // Eigen::Vector2f deltaAcc = factorP * deltaPos + factorV * deltaVel;
        // this->acceleration = this->acceleration + deltaAcc;
        this->velocity += (avgVel) * matchingFactor;
        this->velocity += (avgPos) * centeringFactor;
    }
}

void Boid::update() {

    this->velocity = this->velocity + this->acceleration * DT;
    if (this->velocity.norm() > MAX_SPEED) {
        this->velocity = this->velocity / MAX_SPEED; 
    } else if (this->velocity.norm() > MIN_SPEED){
        this->velocity = this->velocity / MIN_SPEED;
    }
    this->position = this->position + this->velocity * DT;
}

std::optional<std::pair<Vector2f, Vector2f>> Boid::getBoidsAvgInfo(const std::vector<Boid>& boids) {
    Vector2f sumPos(0, 0);
    Vector2f sumVel(0, 0);
    int count = 0;
    const float perceptionRadius = 50.0f;
    const float perceptionRadiusSqr = perceptionRadius*perceptionRadius;

    for (const auto& other : boids) {
        Vector2f deltaP = other.position - this->position;
        Vector2f deltaV = other.velocity - this->velocity;
        const float dist = deltaP.norm();
        if (dist > 0) {
            float velW = fmax(0, (perceptionRadius - dist) / perceptionRadius);
            float posW = fmax(0, (perceptionRadius - dist) / perceptionRadius);
            sumPos += deltaP;
            sumVel += deltaV;
        }
        
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

