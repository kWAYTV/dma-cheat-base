#pragma once
#include <Pch.h>

using json = nlohmann::json;

namespace Config
{
    struct ProcConfig {
        std::string Name;
        std::string Base;
    };

    struct KmboxConfig {
        std::string Ip;
        unsigned short Port;
        std::string Uuid;
    };

    struct AppConfig {
        ProcConfig Proc;
        KmboxConfig Kmbox;

        bool CheckFileExists(const std::string& filename)
        {
            if (!std::filesystem::exists(filename)) {
                std::ofstream file(filename);
                if (file.is_open()) {
                    json j;
                    j["Proc"]["Name"] = "";
                    j["Proc"]["Base"] = "";
                    j["Kmbox"]["Ip"] = "";
                    j["Kmbox"]["Port"] = 0;
                    j["Kmbox"]["Uuid"] = "";

                    file << j.dump(4);  // Write JSON with pretty print
                    file.close();
                    WARN("Created default config file: {}", filename);
                }
                else {
                    ERROR("Could not create config file: {}", filename);
                }
                return false;
            }
            return true;
        }

        void LoadFromFile(const std::string& filename)
        {
            std::ifstream file(filename);
            if (file.is_open()) {
                json j;
                file >> j;

                if (j.contains("Proc")) {
                    if (j["Proc"].contains("Name")) Proc.Name = j["Proc"]["Name"];
                    if (j["Proc"].contains("Base")) Proc.Base = j["Proc"]["Base"];
                }

                if (j.contains("Kmbox")) {
                    if (j["Kmbox"].contains("Ip")) Kmbox.Ip = j["Kmbox"]["Ip"];
                    if (j["Kmbox"].contains("Port")) Kmbox.Port = j["Kmbox"]["Port"];
                    if (j["Kmbox"].contains("Uuid")) Kmbox.Uuid = j["Kmbox"]["Uuid"];
                }

                INFO("Loaded config from file: {}", filename);
            }
            else {
                ERROR("Could not open config file: {}", filename);
            }
        }
    };
}
