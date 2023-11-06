#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include <iostream>
#include <cmath>
#include <random>
#include <gflags/gflags.h>
// #include "external/quad/Quadtree.h"
#include "external/rapidjson/rapidjson.h"
#include "external/rapidjson/document.h"
#include "external/rapidjson/istreamwrapper.h"
#include <fstream>

#include "world/world.h"
#include "world/boid.h"
#include "world/geometry.h"

DEFINE_string(config_path, "config.json", "path to config");



const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float MAX_SPEED = 25.0f;
const float DT = 0.01f;

void readJSONConfig(const std::string& filepath, rapidjson::Document& doc) {
    std::ifstream file(filepath);
    rapidjson::IStreamWrapper isw(file);
    doc.ParseStream(isw);
}

World::WorldConfig readConfig(rapidjson::Document &config_json) {

    World::WorldConfig world_config;
    if (config_json.HasMember("world")) {
        const rapidjson::Value& world_json = config_json["world"];

        if (world_json.HasMember("width")) {
            world_config.width = world_json["width"].GetInt();
        }
        if (world_json.HasMember("height")) {
            world_config.height = world_json["height"].GetInt();
        }
        if (world_json.HasMember("margin")) {
            world_config.margin= world_json["margin"].GetInt();
        }
        if (world_json.HasMember("dt")) {
            world_config.margin= world_json["dt"].GetInt();
        }
    }    
    return world_config;
}

void boid_draw(sf::RenderWindow& window, Boid boid) {
    sf::CircleShape shape(1);  // simple circle for each boid
    sf::Vector2f pos = sf::Vector2f(boid.getPosition().x(), boid.getPosition().y());
    shape.setPosition(pos);
    window.draw(shape);
}

int main(int argc, char* argv[]) {
    // todo(maor): move to config 
    int width;
    int height;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    std::cout << "Config: " << FLAGS_config_path << std::endl;
    rapidjson::Document config_json;
    readJSONConfig(FLAGS_config_path, config_json);
    World::WorldConfig world_config = readConfig(config_json);

    std::cout << "window " << world_config.width << std::endl;

    sf::RenderWindow window(sf::VideoMode(world_config.width, world_config.height), "Swarm Behavior");

    World world = World(world_config);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        world.update();

        window.clear();

        std::vector<Boid> boids = world.getBoids();
        for (auto& boid : boids) {
            boid_draw(window, boid);
            // std::cout << boid.getPosition().x() << std::endl;  
        }
        window.display();
    }

    return 0;
}
