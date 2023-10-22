#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include <iostream>
#include <cmath>
#include <random>
#include <gflags/gflags.h>
#include "external/quad/Quadtree.h"
#include "external/rapidjson/rapidjson.h"
#include "external/rapidjson/document.h"
#include "external/rapidjson/istreamwrapper.h"
#include <fstream>
#include "world/Boid.h"

DEFINE_string(config_path, "config.json", "path to config");



const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float MAX_SPEED = 25.0f;
const float DT = 0.01f;

using namespace quadtree;

class Boid {
private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    float touchRadius;
    float sensingRadius;

    static inline const float maxSpeed = 4.0f; // C++17 inline variable

public:
    Boid(float x, float y, float vx, float vy) : Boid(x, y) {
        this->velocity = sf::Vector2f(vx, vy);
    }

    Boid(float x, float y) : position(x, y), velocity(0, 0), acceleration(0, 0) {
        touchRadius = 2.5;
        sensingRadius = 50;
        box.left = x - touchRadius;
        box.top = y - touchRadius;
        box.width = 2.0 * touchRadius;
        box.height = 2.0 * touchRadius;
    }

    float getX() const {
        return this->position.x;
    }

    void setX(float x) {
        this->position.x = x;
        this->box.left = x - this->box.width * 0.5f;
    }
    
    float getY() const {
        return this->position.y;
    }

    void setY(float y) {
        this->position.y = y;
        this->box.top = y - this->box.height * 0.5f;
    }
    
    Box<float> box;
    
    template <typename GetBoxFunc>
    std::vector<Boid> getNeighbors(float radius, const Quadtree<Boid*, GetBoxFunc>& quadtree) const {

        float x = this->getX();
        float y = this->getY();

        Box qbox = Box<float>(x - radius,
                              y - radius,
                              2.0 * radius,
                              2.0 * radius);
        // std::cout << "[" << qbox.left << ", " << qbox.left + qbox.width << "], [" << qbox.top << ", " << qbox.top + qbox.height << "]" << std::endl; 
        std::vector<Boid*> neighbors = quadtree.query(qbox);
        // std::cout << " > " << neighbors.size() << std::endl;
        std::vector<Boid> ngh = std::vector<Boid>();
        for (auto neighbor : neighbors) {
            if ((this->box.left - neighbor->box.left) * (this->box.left - neighbor->box.left) + 
                (this->box.top - neighbor->box.top) * (this->box.top - neighbor->box.top) < 
                radius * radius) {
                ngh.push_back(*neighbor);
            }
        }
        return ngh;
    }

    void updateAvoidanceDirection(const std::vector<Boid>& neighbors) {
        sf::Vector2f repulsionForce = sf::Vector2f(0, 0); 
        sf::Vector2f predictiveAvoidance = sf::Vector2f(0, 0);

        float repulsion_factor = 10.0f;

        for (Boid neighbor : neighbors) {
            // Calculate distance to neighbor
            sf::Vector2f delta = this->position - neighbor.position;
            float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y); 

            // Repulsion Force
            if (distance < this->box.width * 0.1) {
                sf::Vector2f away = delta;  // vector from neighbor to boid
                away = away * (std::sqrt(away.x * away.x + away.y * away.y));   // Make it a unit vector
                away /= (distance + 0.0001f);   // Weaker force if further away
                repulsionForce += away;
            }

            // Predictive Avoidance
            // sf::Vector2f futureBoidPosition = this->position + boid.velocity * PREDICTIVE_LOOKAHEAD;
            // sf::Vector2f futureNeighborPosition = neighbor->position + neighbor->velocity * PREDICTIVE_LOOKAHEAD;

            // if (futureBoidPosition.distanceTo(futureNeighborPosition) < COLLISION_THRESHOLD) {
            //     Vector2D avoidanceDir = futureBoidPosition - futureNeighborPosition;
            //     avoidanceDir.normalize();
            //     predictiveAvoidance += avoidanceDir;
            // }
        }

        // Combine forces and normalize
        sf::Vector2f combined = repulsionForce + predictiveAvoidance;
        // if (combined.length() > 0) {
        //     combined.normalize();
        // std::cout << "old: " << this->velocity.x << ", " << this->velocity.y;
        this->velocity = this->velocity + repulsion_factor * combined;
        // std::cout << "--> new: " << this->velocity.x << ", " << this->velocity.y << std::endl;
        // this->velocity += 0.000001f * combined;
        // }
    }


    template <typename GetBoxFunc>
    void update(const Quadtree<Boid*, GetBoxFunc>& quadtree) {
        float dt = 0.01;

        // std::cout << "old: [" << this->getX() << ", " << this->getY() << "]";  

        if (this->getX() < 0) {
            this->setX(-1.0 * this->getX());
            velocity.x = -1.0 * velocity.x;
        } 
        if (this->getX() > 800) {
            this->setX(2 * 800.0 - 1.0 * this->getX());
            velocity.x = -velocity.x;
        } 
        if (this->getY() < 0) {
            this->setY(-1.0 * this->getY());
            velocity.y = -1.0 * velocity.y;
        } 
        if (this->getY() > 600) {
            this->setY(2 * 600.0 - 1.0 * this->getY());
            velocity.y = -1.0 * velocity.y;
        } 

        // std::cout << " --> new:[" << this->getX() << ", " << this->getY() << "]" << std::endl; 

        std::vector<Boid> touching = this->getNeighbors(this->box.width, quadtree);
        this->updateAvoidanceDirection(touching);


        std::vector<Boid> neighbors = this->getNeighbors(50.0f, quadtree);
        std::cout << neighbors.size() << std::endl;
        auto result = this->getBoidsAvgInfo(neighbors);

        
        if (result) {
            sf::Vector2f avgPos = result->first;
            sf::Vector2f avgVel = result->second;
            // Use avgPos and avgVel as needed

            sf::Vector2f deltaPos = avgPos - this->position; 
            sf::Vector2f deltaVel = avgVel - this->velocity; 
            float factorAcc = 0.01f;
            float factorPos = 0.01f;

            sf::Vector2f deltaAcc = factorPos * deltaPos + factorAcc * deltaVel;

            this->acceleration = this->acceleration + deltaAcc;
        }
        else {
            // this->velocity = 0.9f * this->velocity;
        }


        this->velocity = this->velocity + this->acceleration * DT;
        this->position = this->position + this->velocity * DT;

        float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        if (speed > MAX_SPEED) {
            velocity = (velocity / speed) * MAX_SPEED;
        }


        // TODO(remove redundency of box and position)
        this->setX(this->getX());
        this->setY(this->getY());
    };
    void draw(sf::RenderWindow& window) {
        sf::CircleShape shape(5);  // simple circle for each boid
        shape.setPosition(position);
        window.draw(shape);
    }

    std::optional<std::pair<sf::Vector2f, sf::Vector2f>> getBoidsAvgInfo(const std::vector<Boid>& boids) {
        sf::Vector2f sumPos(0, 0);
        sf::Vector2f sumVel(0, 0);
        int count = 0;
        const float perceptionRadius = 50.0f;
        const float perceptionRadiusSqr = perceptionRadius*perceptionRadius;

        for (const auto& other : boids) {
            sf::Vector2f delta = other.position - this->position;
            const float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            if (dist > 0) {
                float velW = fmax(0, (perceptionRadius - dist) / perceptionRadius);
                float posW = fmax(0, (perceptionRadius - dist) / perceptionRadius);
                sumPos += posW * delta;
                sumVel += velW * other.velocity;
            }
         
            count++;

        }
        if (count > 0) {
            return std::make_pair(
                this->position + (sumPos / float(count)), 
                sumVel / float(count)
            );
        } else {
            return {};  // No neighbors found
        }
    }

};


auto getBox = [](Boid* boid)
{
    return boid->box;
};

// template <typename GetBoxFunc>
// std::vector<Boid> getNeighbors(const Boid& boid, float radius, const Quadtree<Boid*, GetBoxFunc>& quadtree) {
//     float r = boid.box.width * 0.5;

//     Box box = Box(boid.box.left + r - radius,
//                   boid.box.top + r - radius,
//                   boid.box.left + r + radius,
//                   boid.box.top + r + radius);
//     std::vector<Boid*> neighbors = quadtree.query(box);
//     std::vector<Boid> ngh = std::vector<Boid>();
//     for (auto neighbor : neighbors) {
//         ngh.push_back(*neighbor);
//     }
//     return ngh;
// }


rapidjson::Document readJSONConfig(const std::string& filepath) {
    std::ifstream file(filepath);
    rapidjson::IStreamWrapper isw(file);
    
    rapidjson::Document doc;
    doc.ParseStream(isw);

    return doc;
}

int main(int argc, char* argv[]) {
    // todo(maor): move to config 
    int width;
    int height;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    std::cout << "Config: " << FLAGS_config_path << std::endl;
    rapidjson::Document config = readJSONConfig(FLAGS_config_path);
    if (config.HasMember("world")) {
        const rapidjson::Value& world = config["world"];

        if (world.HasMember("width")) {
            width = world["width"].GetInt();
            std::cout << width << std::endl;
        }
        if (world.HasMember("height")) {
            height = world["height"].GetInt();
        }
    }

    sf::RenderWindow window(sf::VideoMode(width, height), "Swarm Behavior");

    auto box = Box(0.0f, 0.0f, float(width), float(height));
    std::vector<Boid> boids;

    auto quadtree = Quadtree<Boid*, decltype(getBox)>(box, getBox);
    auto border = 10;

    std::random_device rd;  // Used to initialize the seed
    std::mt19937 gen(rd()); // Mersenne Twister pseudo-random generator
    std::uniform_real_distribution<> dis(0.0, 1.0); // Uniform distribution between 0.0 and 1.0

    for (int i = 0; i < 300; i++) {
        float dir = dis(gen) * 2.0f * M_PI;
        float v0 = dis(gen) * MAX_SPEED;
        float vx = std::sin(dir) * v0;
        float vy = std::cos(dir) * v0;

        
        boids.emplace_back(border + int(dis(gen) * width * 10.0f) % (width - 2* border), 
                           border + int(dis(gen) * height * 10.0f) % (height - 2* border),
                           vx, vy);
    }
    for (auto& boid : boids)
        quadtree.add(&boid);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        for (auto& boid : boids) {
            // auto neighbors = getNeighbors(boid, 50, quadtree);
            // std::cout << neighbors.size();
            boid.update(quadtree);
        }

        window.clear();
        for (auto& boid : boids) {
            boid.draw(window);
        }
        window.display();
    }

    return 0;
}
