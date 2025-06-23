// ConfigParser.cpp - Implementation for the configuration file parser.

#include "ConfigParser.h"
#include <fstream>
#include <iostream>
#include <algorithm>

void ConfigParser::trim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

void ConfigParser::process_line(const std::string& line, AppConfig& config) {
    std::string temp = line;
    trim(temp);
    
    if (temp.empty() || temp[0] == '#') {
        return; // Skip comments and empty lines
    }

    if (temp[0] == '[' && temp.back() == ']') {
        // This is a section header, e.g., [Plugins]
        m_current_section = temp.substr(1, temp.length() - 2);
        return;
    }

    auto delimiter_pos = temp.find('=');
    if (delimiter_pos == std::string::npos) {
        std::cerr << "Warning: Malformed line in config: " << line << std::endl;
        return;
    }

    std::string key = temp.substr(0, delimiter_pos);
    std::string value = temp.substr(delimiter_pos + 1);
    trim(key);
    trim(value);
    
    if (m_current_section == "Core") {
        if (key == "log_file_path") config.log_file_path = value;
        else if (key == "log_level") config.log_level = std::stoi(value);
        else if (key == "worker_threads") config.worker_threads = std::stoul(value);
        else if (key == "memory_pool_size_mb") config.memory_pool_size_mb = std::stoul(value);
        else if (key == "simulation_timestep") config.simulation_timestep = std::stod(value);
    } else if (m_current_section == "Plugins") {
        // Try to guess the type for variant
        try {
            config.plugin_settings[key] = std::stod(value);
        } catch (...) {
            try {
                config.plugin_settings[key] = std::stoi(value);
            } catch (...) {
                config.plugin_settings[key] = value;
            }
        }
    }
}

AppConfig ConfigParser::parse(const std::string& file_path) {
    AppConfig config;
    std::ifstream file(file_path);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open config file: " << file_path << std::endl;
        config.is_valid = false;
        return config;
    }

    m_current_section = "Core"; // Default section
    std::string line;
    while (std::getline(file, line)) {
        process_line(line, config);
    }
    
    config.is_valid = true;
    return config;
}
