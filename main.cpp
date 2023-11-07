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
#include "aux/config.h"

DEFINE_string(config_path, "config.json", "path to config");

static World::WorldConfig ParseWorldConfig(rapidjson::Document &config_json) {

    World::WorldConfig world_config;
    if (config_json.HasMember("world")) {
        const rapidjson::Value& world_json = config_json["world"];

        world_config.width = Config::GetConfigValue<int>(world_json, "width", 800);
        world_config.height = Config::GetConfigValue<int>(world_json, "height", 600);
        world_config.margin = Config::GetConfigValue<int>(world_json, "margin", 10);
        world_config.dt = Config::GetConfigValue<float>(world_json, "dt", 0.1f);
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
    std::cout << "Config file: " << FLAGS_config_path << std::endl;
    rapidjson::Document config_json;
    Config::readJSONConfig(FLAGS_config_path, config_json);
    World::WorldConfig world_config = ParseWorldConfig(config_json);

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
        }
        window.display();
    }

    return 0;
}
