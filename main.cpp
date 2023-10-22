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

// auto getBox = [](Boid* boid)
// {
//     return boid->box;
// };

struct GetBoxFunctor {
    Box<float> operator()(const Boid *boid) const {
        return boid->getBox();
    }
};

rapidjson::Document readJSONConfig(const std::string& filepath) {
    std::ifstream file(filepath);
    rapidjson::IStreamWrapper isw(file);
    
    rapidjson::Document doc;
    doc.ParseStream(isw);

    return doc;
}

void boid_draw(sf::RenderWindow& window, Boid boid) {
    sf::CircleShape shape(5);  // simple circle for each boid
    sf::Vector2f pos = sf::Vector2f(boid.getX(), boid.getY());
    shape.setPosition(pos);
    window.draw(shape);
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

    GetBoxFunctor getBoxFunctor;
    auto quadtree = Quadtree<Boid*,GetBoxFunctor>(box, getBoxFunctor);
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
            boid_draw(window, boid);
        }
        window.display();
    }

    return 0;
}
