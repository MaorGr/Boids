#pragma once

#include "geometry.h"

class Boid {
private:
    unsigned int id; 
    static int maxId;
    Eigen::Vector2f position;
    Eigen::Vector2f velocity;
    Eigen::Vector2f acceleration;
    float touchRadius;
    Box bbox;
    // from config
    int type_id;
    float max_speed;
    float min_speed;
    float avoid_radius;
    float sense_radius;
    float avoid_factor;
    float matching_factor;
    float centering_factor;

public:
    struct BoidConfig {
        int type_id;
        float max_speed;
        float min_speed;
        float avoid_radius;
        float sense_radius;
        float avoid_factor;
        float matching_factor;
        float centering_factor;
        
    };

    BoidConfig config_;

    Boid(float x, float y, float vx, float vy, const BoidConfig& config);
    Vector2f getPosition();
    void setPosition(Vector2f position);
    Vector2f getVelocity();
    void setVelocity(Vector2f velocity);
    float getSenseRadius();
    void setSenseRadius(float velocity);
    float getAvoidRadius();
    void setAvoidRadius(float velocity);
    Box getBox();
    inline unsigned int getId() { return id; };

    // quadtree::Box <float> box;
    // quadtree::Box <float> getBox() const;
    
    // template <typename GetBoxFunc>
    // std::vector<Boid> getNeighbors(float radius, const quadtree::Quadtree<Boid*, GetBoxFunc>& quadtree) const;

    void doFlocking(std::vector<Boid> &neighbors);
    void updateAvoidanceDirection(const std::vector<Boid>& neighbors);

    // // template <typename GetBoxFunc>
    // void update(const quadtree::Quadtree<Boid*, GetBoxFunctor>& quadtree);
    void update(float dt);
    std::optional<std::pair<Vector2f, Vector2f>> getBoidsAvgInfo(const std::vector<Boid>& boids);

};