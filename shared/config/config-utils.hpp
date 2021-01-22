#ifndef CONFIG_UTILS_H
#define CONFIG_UTILS_H
// Provides helper functions for configuration.

#include <optional>
#include <stdio.h>
#include <stdlib.h>
#include "../utils/utils-functions.h"
#include "rapidjson-utils.hpp"
#include "modloader/shared/modloader.hpp"
#include <string>

// typedef rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator> ConfigDocument;
// typedef rapidjson::GenericValue<rapidjson::UTF8<>, rapidjson::CrtAllocator> ConfigValue;
typedef rapidjson::Document ConfigDocument;
typedef rapidjson::Value ConfigValue;

#ifndef PERSISTENT_DIR
#define PERSISTENT_DIR "/sdcard/ModData/%s/Mods/"
#endif
#ifndef CONFIG_PATH_FORMAT
#define CONFIG_PATH_FORMAT "/sdcard/ModData/%s/Configs/"
#endif

// You are responsible for Loading and Writing to it as necessary.
class Configuration {
public:
    // Returns the config path for the given mod info
    static std::string getConfigFilePath(const ModInfo& info);
    const ModInfo info;
    ConfigDocument config;
    bool readJson = false;
    Configuration(const ModInfo& info_) : info(info_) {
        filePath = Configuration::getConfigFilePath(info_);
    }
    Configuration(Configuration&& other) :
        info(std::move(other.info)),
        filePath(std::move(other.filePath))
    {
        config.Swap(other.config);
    }
    Configuration(const Configuration& other) :
        info(other.info),
        filePath(other.filePath)
    {
        config.CopyFrom(other.config, config.GetAllocator());
    }
    // Loads JSON config
    void Load();
    // Reloads JSON config
    void Reload();
    // Writes JSON config
    void Write();
private:
    static std::optional<std::string> configDir;
    bool ensureObject();
    std::string filePath;
};

// SETTINGS
// ParseError is thrown when failing to parse a JSON file
typedef enum ParseError {
    PARSE_ERROR_FILE_DOES_NOT_EXIST = -1
} ParseError_t;

// WriteError is thrown when failing to create a file
typedef enum WriteError {
    WRITE_ERROR_COULD_NOT_MAKE_FILE = -1
} WriteError_t;

// JSON Parse Errors
typedef enum JsonParseError {
    JSON_PARSE_ERROR = -1
} JsonParseError_t;

// CONFIG
// Parses the JSON of the filename, and returns whether it succeeded or not
static bool parsejsonfile(rapidjson::Document& doc, std::string_view filename);
// Parses a JSON string, and returns whether it succeeded or not
static bool parsejson(ConfigDocument& doc, std::string_view js);

/// @brief Returns a path to the persistent data directory for the provided const ModInfo&.
/// @param info The const ModInfo& to find a path for.
/// @return The path to the directory.
static std::string getDataDir(const ModInfo& info);

#endif /* CONFIG_UTILS_H */
