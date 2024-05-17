#pragma once
#include <Pch.h>

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

                INFO("Loading config from file: {}", filename);
            }
            else {
                ERROR("Could not open config file: {}", filename);
            }
        }
    };
}
