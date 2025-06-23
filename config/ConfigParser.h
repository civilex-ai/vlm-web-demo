// ConfigParser.h - Parses the main application configuration file.

#pragma once

#include <string>
#include <map>
#include <variant>

// A structure to hold the parsed configuration.
struct AppConfig {
    bool is_valid = false;
    std::string log_file_path = "/var/log/app.log";
    int log_level = 2; // 0=Debug, 1=Info, 2=Warn, 3=Error
    size_t worker_threads = 4;
    size_t memory_pool_size_mb = 256;
    double simulation_timestep = 0.016;
    
    // A map for arbitrary plugin settings
    std::map<std::string, std::variant<int, double, std::string>> plugin_settings;
};

class ConfigParser {
public:
    ConfigParser() = default;

    /**
     * @brief Parses a configuration file from the given path.
     * The format is a simple key=value format. Lines starting with '#' are comments.
     * @param file_path The path to the configuration file.
     * @return An AppConfig struct populated with values.
     */
    AppConfig parse(const std::string& file_path);

private:
    void trim(std::string& s);
    void process_line(const std::string& line, AppConfig& config);
    
    // Internal state to track parsing context, e.g., current section.
    std::string m_current_section;
};
