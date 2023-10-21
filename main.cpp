#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include <iostream>
#include <gflags/gflags.h>
#include "external/quad/Quadtree.h"
#include "external/rapidjson/rapidjson.h"
#include "external/rapidjson/document.h"
#include "external/rapidjson/istreamwrapper.h"
#include <fstream>
// #include <external/rapidjson/document.h>
// #include <external/rapidjson/istreamwrapper.h>

DEFINE_string(config_path, "config.json", "path to config");



const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float MAX_SPEED = 100.0f;
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
    Boid(float x, float y) : position(x, y), velocity(0, 0), acceleration(0, 0) {
        touchRadius = 2.5;
        sensingRadius = 50;
        box.left = x - touchRadius;
        box.top = y - touchRadius;
        box.width = 2.0 * touchRadius;
        box.height = 2.0 * touchRadius;
    }

    float getX() {
        return this->position.x;
    }

    void setX(float x) {
        this->position.x = x;
    }
    
    Box<float> box;
    

    template <typename GetBoxFunc>
    std::vector<Boid> getNeighbors(float radius, const Quadtree<Boid*, GetBoxFunc>& quadtree) const {
        float r = box.width * 0.5;

        Box qbox = Box(box.left + r - radius,
                       box.top + r - radius,
                       box.left + r + radius,
                       box.top + r + radius);
        std::vector<Boid*> neighbors = quadtree.query(qbox);
        std::vector<Boid> ngh = std::vector<Boid>();
        for (auto neighbor : neighbors) {
            if (neighbor->box.left * neighbor->box.left + neighbor->box.top * neighbor->box.top > radius * radius) {
                ngh.push_back(*neighbor);
            }
        }
        return ngh;
    }

    template <typename GetBoxFunc>
    void update(const Quadtree<Boid*, GetBoxFunc>& quadtree) {
        float dt = 0.01;
        // auto result = this->getNeighboringBoidsAvgInfo(allBoids);
        std::vector<Boid> neighbors = this->getNeighbors(50, quadtree);
        auto result = this->getBoidsAvgInfo(neighbors);
        
        if (result) {
            sf::Vector2f avgPos = result->first;
            sf::Vector2f avgVel = result->second;
            // Use avgPos and avgVel as needed

            sf::Vector2f deltaPos = sf::Vector2f(
                avgPos.x - this->position.x, 
                avgPos.y - this->position.y);
            sf::Vector2f deltaVel = sf::Vector2f(
                avgVel.x - this->velocity.x, 
                avgVel.y - this->velocity.y);

            float factorAcc = 0.01;
            float factorPos = 0.002;

            sf::Vector2f deltaAcc = sf::Vector2f(
                factorPos * deltaPos.x + factorAcc * deltaVel.x,
                factorPos * deltaPos.y + factorAcc * deltaVel.y
            );

            this->acceleration.x = this->acceleration.x + deltaAcc.x;
            this->acceleration.y = this->acceleration.y + deltaAcc.y;

        }
        else {
            this->velocity.x = 0.9 * this->velocity.x;
            this->velocity.y = 0.9 * this->velocity.y;
        }

        std::cout << "old: " << this->position.x;  

        this->velocity.x = this->velocity.x + this->acceleration.x * DT;
        this->velocity.y = this->velocity.y + this->acceleration.y * DT;
        this->position.x = this->position.x + this->velocity.x * DT;
        this->position.y = this->position.y + this->velocity.y * DT;

        float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        if (speed > MAX_SPEED) {
            velocity.x = (velocity.x / speed) * MAX_SPEED;
            velocity.y = (velocity.y / speed) * MAX_SPEED;
        }

        if (position.x < 0) {
            position.x = -1.0 * position.x;
            velocity.x = -1.0 * velocity.x;
        } 
        if (position.x > 800) {
            position.x = 800.0 - position.x;
            velocity.x = -velocity.x;
        } 
        if (position.y < 0) {
            position.y = -1.0 * position.y;
            velocity.y = -1.0 * velocity.y;
        } 
        if (position.y > 600) {
            position.y = 600.0 - position.y;
            velocity.y = -velocity.y;
        } 
        std::cout << "new: " << this->position.x << std::endl; 
        // TODO(remove redundency of box and position)
        box.left = position.x;
        box.top = position.y;
        

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
            sumPos += other.position;
            sumVel += other.velocity;
            count++;

        }

        if (count > 0) {
            return std::make_pair(
                sf::Vector2f(sumPos.x / count, sumPos.y / count), 
                sf::Vector2f(sumVel.x / count, sumVel.y / count)
            );
        } else {
            return {};  // No neighbors found
        }
    }

};


// auto getBox = [](Boid* boid)
// {
//     return boid->box;
// };

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
    for (int i = 0; i < 300; i++) {
        boids.emplace_back(border + rand() % (width - 2* border), 
                           border + rand() % (height - 2* border));
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
