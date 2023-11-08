#pragma once
#include <string>
#include "../external/rapidjson/rapidjson.h"
#include "../external/rapidjson/document.h"
#include "../external/rapidjson/istreamwrapper.h"
#include <fstream>

class Config {
public:
    // Method to load and parse the configuration file, then return a WorldConfig object
    static void readJSONConfig(const std::string& configFilePath, rapidjson::Document& doc) {
        std::cout << "hi" << std::endl;
        std::ifstream file(configFilePath);
        rapidjson::IStreamWrapper isw(file);
        doc.ParseStream(isw);
        std::cout << "bye"; 
    }

    // Helper method to extract configuration values
    template<typename T>
    static T GetConfigValue(const rapidjson::Value& value, const char* memberName, const T& defaultValue) {
        if (value.HasMember(memberName) && value[memberName].Is<T>()) {
            return value[memberName].Get<T>();
        }
        return defaultValue;
    }

    // // Method to parse the "world" section of the config file
    // static WorldConfig ParseWorldConfig(const rapidjson::Document& config_json) {
    //     WorldConfig world_config;
    //     if (config_json.HasMember("world")) {
    //         const rapidjson::Value& world_json = config_json["world"];

    //         world_config.width = GetConfigValue<int>(world_json, "width", 800);
    //         world_config.height = GetConfigValue<int>(world_json, "height", 600);
    //         world_config.margin = GetConfigValue<int>(world_json, "margin", 10);
    //         world_config.dt = GetConfigValue<float>(world_json, "dt", 0.1f);
    //         // ... other parameters ...
    //     }
    //     return world_config;
    // }
};

// Usage:
// Config::WorldConfig myConfig = Config::LoadConfig("path_to_config_file.json");
