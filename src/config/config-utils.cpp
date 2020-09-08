#ifndef CONFIG_DEFINED_H
#define CONFIG_DEFINED_H

#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../../shared/config/config-utils.hpp"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "modloader/shared/modloader.hpp"

// CONFIG

bool readJson = false;
decltype(Configuration::configDir) Configuration::configDir;

using namespace rapidjson;

bool Configuration::ensureObject() {
    if (!config.IsObject()) {
        Logger::get().warning("Config data for mod %s was invalid! Clearing.");
        config.SetObject();
        return false;
    }
    return true;
}

// Loads the config for the given mod, if it doesn't exist, will leave it as an empty object.
void Configuration::Load() {
    if (readJson) {
        return;
    }

    if (!fileexists(filePath)) {
        writefile(filePath, "{}");
    }
    Configuration::Reload();
}

void Configuration::Reload() {
    readJson = parsejsonfile(config, filePath);
    ensureObject();
}

void Configuration::Write() {
    ensureObject();

    StringBuffer buf;
    PrettyWriter<StringBuffer> writer(buf);
    config.Accept(writer);
    writefile(filePath, buf.GetString());
}

bool parsejsonfile(ConfigDocument& doc, std::string_view filename) {
    if (!fileexists(filename.data())) {
        return false;
    }
    std::ifstream is;
    is.open(filename.data());

    IStreamWrapper wrapper {is};
    
    return !doc.ParseStream(wrapper).HasParseError();
}

bool parsejson(ConfigDocument& doc, std::string_view js) {
    char temp[js.length() + 1];
    memcpy(temp, js.data(), js.length());
    temp[js.length()] = '\0';

    if (doc.ParseInsitu(temp).HasParseError()) {
        return false;
    }
    return true;
}

std::string Configuration::getConfigFilePath(const ModInfo& info) {
    if (!Configuration::configDir) {
        #ifdef __aarch64__
        Configuration::configDir = string_format(CONFIG_PATH_FORMAT, Modloader::getApplicationId().c_str());
        #else
        Configuration::configDir = string_format(CONFIG_PATH_FORMAT, PACKAGE_ID);
        #endif
        if (!direxists(Configuration::configDir->c_str())) {
            mkpath(Configuration::configDir->data(), 0700);
        }
    }
    return *Configuration::configDir + info.id + ".json";
}

#endif /* CONFIG_DEFINED_H */