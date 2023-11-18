#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include <iostream>
#include <cmath>
#include <random>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include "external/rapidjson/rapidjson.h"
#include "external/rapidjson/document.h"
#include "external/rapidjson/istreamwrapper.h"
#include <fstream>

#include "world/world.h"
#include "world/boid.h"
#include "world/geometry.h"
#include "aux/config.h"
#include "tracker/tracker.h"

DEFINE_string(config_path, "config.json", "path to config");

static World::WorldConfig ParseWorldConfig(rapidjson::Document &config_json) {

    World::WorldConfig world_config;
    if (config_json.HasMember("world")) {
        const rapidjson::Value& world_json = config_json["world"];

        world_config.width = Config::GetConfigValue<int>(world_json, "width", 800);
        world_config.height = Config::GetConfigValue<int>(world_json, "height", 600);
        world_config.margin = Config::GetConfigValue<float>(world_json, "margin", 100);
        world_config.dt = Config::GetConfigValue<float>(world_json, "dt", 0.1f);
        world_config.boid_count = Config::GetConfigValue<int>(world_json, "boid_count", 1000);
        world_config.turn_factor = Config::GetConfigValue<float>(world_json, "turn_factor", 0.2f);
        world_config.potential_path = Config::GetConfigValue<std::string>(world_json, "potential", "");
        LOG(INFO) << "world_config.potential" << world_config.potential_path << std::endl;
    }
    return world_config;
}

static Boid::BoidConfig ParseBoidConfig(rapidjson::Document &config_json) {

   Boid::BoidConfig boid_config;
    if (config_json.HasMember("boid")) {
        const rapidjson::Value& boid_json = config_json["boid"];

        boid_config.max_speed = Config::GetConfigValue<float>(boid_json, "max_speed", 6);
        boid_config.min_speed = Config::GetConfigValue<float>(boid_json, "min_speed", 3);
        boid_config.avoid_radius = Config::GetConfigValue<float>(boid_json, "avoid_radius", 8);
        boid_config.sense_radius = Config::GetConfigValue<float>(boid_json, "sense_radius", 40);
        boid_config.avoid_factor = Config::GetConfigValue<float>(boid_json, "avoid_factor", 0.05);
        boid_config.matching_factor = Config::GetConfigValue<float>(boid_json, "matching_factor", 0.05);
        boid_config.centering_factor = Config::GetConfigValue<float>(boid_json, "centering_factor", 0.0005);
    }
    return boid_config;
}

void boid_draw(sf::RenderWindow& window, Boid boid) {
    sf::CircleShape shape(1);  // simple circle for each boid
    sf::Vector2f pos = sf::Vector2f(boid.getPosition().x(), boid.getPosition().y());
    shape.setPosition(pos);
    window.draw(shape);
}

int main(int argc, char* argv[]) {
    // todo(maor): move to config 

    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    // TODO(maor): find a nicer way?
    // FLAGS_minloglevel = 0;
    // FLAGS_alsologtostderr = true;
    // FLAGS_log_dir = "/Users/maorgrinberg/Code/boids101/logs";
    FLAGS_logtostderr = true;
    int width;
    int height;
    std::cout << "Config file: " << FLAGS_config_path << std::endl;
    rapidjson::Document config_json;
    Config::readJSONConfig(FLAGS_config_path, config_json);
    World::WorldConfig world_config = ParseWorldConfig(config_json);
    Boid::BoidConfig boid_config = ParseBoidConfig(config_json);

    World world = World(world_config);
    world.populate(boid_config);

    std::string boid_paths;
    if (config_json.HasMember("out")) {
        const rapidjson::Value& out_coufig = config_json["out"];
        boid_paths = Config::GetConfigValue<std::string>(out_coufig, "boid_paths", "data/boid_paths.csv");
    }
    LOG(INFO) << "writing paths to " << boid_paths << std::endl;

    Tracker tracker(boid_paths, world, 3);

    sf::RenderWindow window(sf::VideoMode(world.getWidth(), world.getHeight()), "Swarm Behavior");
    int step = 0;
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
        step += 1;

        tracker.update(world);
        LOG(INFO) << "step: " << step << std::endl;
    }

    return 0;
}
