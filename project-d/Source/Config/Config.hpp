#pragma once
#include "Structs.hpp"

namespace Config
{
    struct AppConfig {
        Structs::AimConfig Aim;
        Structs::KmboxConfig Kmbox;
        Structs::VisualsConfig Visuals;

        static AppConfig& Get()
        {
            static AppConfig instance;
            return instance;
        }

        bool Init(const std::string& configFile = "config.json")
        {
            std::string configDir = "configs";
            if (!std::filesystem::exists(configDir))
            {
                if (!std::filesystem::create_directory(configDir))
                {
                    LOG_ERROR("Could not create config directory: {}", configDir);
                    return false;
                }
            }

            std::string fullPath = configDir + "/" + configFile;
            if (!CheckFileExists(fullPath))
            {
                LOG_ERROR("Configuration initialization failed. If you did not have one, fill out the newly created file.");
                return false;
            }

            if (!LoadFromFile(fullPath))
            {
                LOG_ERROR("Couldn't load the specified config file");
                return false;
            }

            return true;
        }

        bool SaveToFile(const std::string& filename)
        {
            std::string fullPath = filename;
            LOG_INFO("Attempting to save config to file: {}", fullPath);
            nlohmann::json j;

            j["Aim"]["Trigger"] = Aim.Trigger;
            j["Aim"]["TriggerKey"] = Aim.TriggerKey;
            j["Aim"]["TriggerKeyMode"] = Aim.TriggerKeyMode;
            j["Aim"]["TriggerDelay"] = Aim.TriggerDelay;

            j["Aim"]["Aimbot"] = Aim.Aimbot;

            j["Aim"]["DrawFov"] = Aim.DrawFov;
            j["Aim"]["AimbotFovColor"] = { Aim.AimbotFovColor.x, Aim.AimbotFovColor.y, Aim.AimbotFovColor.z, Aim.AimbotFovColor.w };

            j["Aim"]["AimFriendly"] = Aim.AimFriendly;
            j["Aim"]["AimVisible"] = Aim.AimVisible;

            j["Aim"]["AimbotKey"] = Aim.AimbotKey;
            j["Aim"]["AimbotKeyMode"] = Aim.AimbotKeyMode;
            j["Aim"]["AimbotFov"] = Aim.AimbotFov;
            j["Aim"]["AimbotSmooth"] = Aim.AimbotSmooth;

            j["Kmbox"]["Enabled"] = Kmbox.Enabled;
            j["Kmbox"]["Ip"] = Kmbox.Ip;
            j["Kmbox"]["Port"] = Kmbox.Port;
            j["Kmbox"]["Uuid"] = Kmbox.Uuid;

            j["Visuals"]["Enabled"] = Visuals.Enabled;
            j["Visuals"]["TeamCheck"] = Visuals.TeamCheck;
            j["Visuals"]["VisibleCheck"] = Visuals.VisibleCheck;

            j["Visuals"]["Background"] = Visuals.Background;
            j["Visuals"]["Hitmarker"] = Visuals.Hitmarker;
            j["Visuals"]["HitmarkerColor"] = { Visuals.HitmarkerColor.x, Visuals.HitmarkerColor.y, Visuals.HitmarkerColor.z, Visuals.HitmarkerColor.w };
            j["Visuals"]["Watermark"] = Visuals.Watermark;
            j["Visuals"]["WatermarkColor"] = { Visuals.WatermarkColor.x, Visuals.WatermarkColor.y, Visuals.WatermarkColor.z, Visuals.WatermarkColor.w };

            j["Visuals"]["Name"] = Visuals.Name;
            j["Visuals"]["NameColor"] = { Visuals.NameColor.x, Visuals.NameColor.y, Visuals.NameColor.z, Visuals.NameColor.w };
            j["Visuals"]["Box"] = Visuals.Box;
            j["Visuals"]["BoxColor"] = { Visuals.BoxColor.x, Visuals.BoxColor.y, Visuals.BoxColor.z, Visuals.BoxColor.w };
            j["Visuals"]["BoxColorVisible"] = { Visuals.BoxColorVisible.x, Visuals.BoxColorVisible.y, Visuals.BoxColorVisible.z, Visuals.BoxColorVisible.w };
            j["Visuals"]["Health"] = Visuals.Health;
            j["Visuals"]["Weapon"] = Visuals.Weapon;
            j["Visuals"]["WeaponColor"] = { Visuals.WeaponColor.x, Visuals.WeaponColor.y, Visuals.WeaponColor.z, Visuals.WeaponColor.w };
            j["Visuals"]["Bones"] = Visuals.Bones;
            j["Visuals"]["BonesColor"] = { Visuals.BonesColor.x, Visuals.BonesColor.y, Visuals.BonesColor.z, Visuals.BonesColor.w };

            std::ofstream file(fullPath);
            if (file.is_open())
            {
                file << j.dump(4);  // Write JSON with pretty print
                file.close();
                LOG_INFO("Config saved successfully to file: {}", fullPath);

                return true;
            }
            else
            {
                LOG_ERROR("Failed to open file for saving: {}", fullPath);
            }

            return false;
        }

        bool DeleteConfigFile(const std::string& filename)
        {
            std::string fullPath = filename;
            LOG_INFO("Attempting to delete config file: {}", fullPath);
            if (std::filesystem::remove(fullPath))
            {
                LOG_INFO("Config file deleted successfully: {}", fullPath);
                return true;
            }
            else
            {
                LOG_ERROR("Failed to delete config file: {}", fullPath);
            }
            return false;
        }

        std::vector<std::string> ListConfigs(const std::string& directory)
        {
            std::vector<std::string> configs;
            for (const auto& entry : std::filesystem::directory_iterator(directory))
            {
                if (entry.is_regular_file() && entry.path().extension() == ".json")
                {
                    configs.push_back(entry.path().filename().string());
                }
            }
            return configs;
        }

        bool LoadFromFile(const std::string& filename)
        {
            std::ifstream file(filename);
            if (file.is_open())
            {
                nlohmann::json j;
                try {
                    file >> j;
                }
                catch (nlohmann::json::parse_error& e) {
                    LOG_ERROR("JSON parse error in config file: {}", e.what());
                    return false;
                }

                LoadConfigSection(j, "Aim", Aim);
                LoadConfigSection(j, "Kmbox", Kmbox);
                LoadConfigSection(j, "Visuals", Visuals);

                LOG_INFO("Loaded config from file: {}", filename);
                return true;
            }
            else
            {
                LOG_ERROR("Could not open config file: {}", filename);
                return false;
            }
        }

        bool LoadFromClipboard()
        {
            if (OpenClipboard(nullptr))
            {
                HANDLE clipboardData = GetClipboardData(CF_TEXT);
                if (clipboardData)
                {
                    char* clipboardText = static_cast<char*>(GlobalLock(clipboardData));
                    if (clipboardText)
                    {
                        try
                        {
                            nlohmann::json j = nlohmann::json::parse(clipboardText);
                            LoadConfigSection(j, "Aim", Aim);
                            LoadConfigSection(j, "Kmbox", Kmbox);
                            LoadConfigSection(j, "Visuals", Visuals);
                            LOG_INFO("Loaded config from clipboard");
                            GlobalUnlock(clipboardData);
                            CloseClipboard();
                            return true;
                        }
                        catch (nlohmann::json::parse_error& e)
                        {
                            LOG_ERROR("JSON parse error in clipboard data: {}", e.what());
                        }
                    }
                    GlobalUnlock(clipboardData);
                }
                CloseClipboard();
            }
            return false;
        }

    private:
        bool CheckFileExists(const std::string& filename)
        {
            if (!std::filesystem::exists(filename))
            {
                if (CreateDefaultConfigFile(filename))
                {
                    LOG_WARN("Created default config file: {}", filename);
                    return true;
                }
                else
                {
                    LOG_ERROR("Could not create config file: {}", filename);
                    return false;
                }
            }
            return true;  // File exists
        }

        bool CreateDefaultConfigFile(const std::string& filename)
        {
            std::ofstream file(filename);
            if (file.is_open())
            {
                nlohmann::json j;

                j["Kmbox"]["Enabled"] = false;
                j["Kmbox"]["Ip"] = "";
                j["Kmbox"]["Port"] = 0;
                j["Kmbox"]["Uuid"] = "";

                j["Aim"]["Trigger"] = false;
                j["Aim"]["TriggerKey"] = 0;
                j["Aim"]["TriggerKeyMode"] = 0;
                j["Aim"]["TriggerDelay"] = 0;

                j["Aim"]["Aimbot"] = false;

                j["Aim"]["DrawFov"] = false;
                j["Aim"]["AimbotFovColor"] = { 1.0f, 1.0f, 1.0f, 1.0f };

                j["Aim"]["AimFriendly"] = false;
                j["Aim"]["AimVisible"] = false;

                j["Aim"]["AimbotKey"] = 0;
                j["Aim"]["AimbotKeyMode"] = 0;
                j["Aim"]["AimbotFov"] = 0.0f;
                j["Aim"]["AimbotSmooth"] = 0.0f;

                j["Visuals"]["Enabled"] = false;
                j["Visuals"]["TeamCheck"] = false;
                j["Visuals"]["VisibleCheck"] = false;

                j["Visuals"]["Background"] = false;
                j["Visuals"]["Hitmarker"] = false;
                j["Visuals"]["HitmarkerColor"] = { 1.0f, 1.0f, 1.0f, 1.0f };
                j["Visuals"]["Watermark"] = false;
                j["Visuals"]["WatermarkColor"] = { 1.0f, 1.0f, 1.0f, 1.0f };

                j["Visuals"]["Name"] = false;
                j["Visuals"]["NameColor"] = { 1.0f, 1.0f, 1.0f, 1.0f };
                j["Visuals"]["Box"] = false;
                j["Visuals"]["BoxColor"] = { 1.0f, 1.0f, 1.0f, 1.0f };
                j["Visuals"]["BoxColorVisible"] = { 1.0f, 1.0f, 1.0f, 1.0f };
                j["Visuals"]["Health"] = false;
                j["Visuals"]["Weapon"] = false;
                j["Visuals"]["WeaponColor"] = { 1.0f, 1.0f, 1.0f, 1.0f };
                j["Visuals"]["Bones"] = false;
                j["Visuals"]["BonesColor"] = { 1.0f, 1.0f, 1.0f, 1.0f };

                file << j.dump(4);  // Write JSON with pretty print
                file.close();
                return true;
            }
            return false;
        }

        template<typename T>
        void LoadConfigSection(const nlohmann::json& j, const std::string& sectionName, T& configSection)
        {
            if (j.contains(sectionName))
            {
                const auto& section = j[sectionName];
                for (const auto& [key, value] : section.items())
                {
                    if (section.contains(key))
                    {
                        if constexpr (std::is_same_v<T, Structs::AimConfig>)
                        {
                            if (key == "Trigger") configSection.Trigger = value.get<bool>();
                            else if (key == "TriggerKey") configSection.TriggerKey = value.get<int>();
                            else if (key == "TriggerKeyMode") configSection.TriggerKeyMode = value.get<int>();
							else if (key == "TriggerDelay") configSection.TriggerDelay = value.get<int>();

                            else if (key == "Aimbot") configSection.Aimbot = value.get<bool>();

                            else if (key == "DrawFov") configSection.DrawFov = value.get<bool>();
                            else if (key == "AimbotFovColor") configSection.AimbotFovColor = ImVec4(value[0].get<float>(), value[1].get<float>(), value[2].get<float>(), value[3].get<float>());

                            else if (key == "AimFriendly") configSection.AimFriendly = value.get<bool>();
                            else if (key == "AimVisible") configSection.AimVisible = value.get<bool>();

                            else if (key == "AimbotKey") configSection.AimbotKey = value.get<int>();
							else if (key == "AimbotKeyMode") configSection.AimbotKeyMode = value.get<int>();
                            else if (key == "AimbotFov") configSection.AimbotFov = value.get<float>();
                            else if (key == "AimbotSmooth") configSection.AimbotSmooth = value.get<float>();
                        }
                        else if constexpr (std::is_same_v<T, Structs::KmboxConfig>)
                        {
                            if (key == "Enabled") configSection.Enabled = value.get<bool>();
                            else if (key == "Ip") configSection.Ip = value.get<std::string>();
                            else if (key == "Port") configSection.Port = value.get<unsigned short>();
                            else if (key == "Uuid") configSection.Uuid = value.get<std::string>();
                        }
                        else if constexpr (std::is_same_v<T, Structs::VisualsConfig>)
                        {
                            if (key == "Enabled") configSection.Enabled = value.get<bool>();
                            else if (key == "TeamCheck") configSection.TeamCheck = value.get<bool>();
                            else if (key == "VisibleCheck") configSection.VisibleCheck = value.get<bool>();

                            else if (key == "Background") configSection.Background = value.get<bool>();
                            else if (key == "Hitmarker") configSection.Hitmarker = value.get<bool>();
                            else if (key == "HitmarkerColor") configSection.HitmarkerColor = ImVec4(value[0].get<float>(), value[1].get<float>(), value[2].get<float>(), value[3].get<float>());
                            else if (key == "Watermark") configSection.Watermark = value.get<bool>();

                            else if (key == "Name") configSection.Name = value.get<bool>();
                            else if (key == "Box") configSection.Box = value.get<bool>();
                            else if (key == "Health") configSection.Health = value.get<bool>();
                            else if (key == "Weapon") configSection.Weapon = value.get<bool>();
                            else if (key == "Bones") configSection.Bones = value.get<bool>();

                            else if (key == "WatermarkColor") configSection.WatermarkColor = ImVec4(value[0].get<float>(), value[1].get<float>(), value[2].get<float>(), value[3].get<float>());
                            else if (key == "NameColor") configSection.NameColor = ImVec4(value[0].get<float>(), value[1].get<float>(), value[2].get<float>(), value[3].get<float>());
                            else if (key == "BoxColor") configSection.BoxColor = ImVec4(value[0].get<float>(), value[1].get<float>(), value[2].get<float>(), value[3].get<float>());
                            else if (key == "BoxColorVisible") configSection.BoxColorVisible = ImVec4(value[0].get<float>(), value[1].get<float>(), value[2].get<float>(), value[3].get<float>());
                            else if (key == "WeaponColor") configSection.WeaponColor = ImVec4(value[0].get<float>(), value[1].get<float>(), value[2].get<float>(), value[3].get<float>());
                            else if (key == "BonesColor") configSection.BonesColor = ImVec4(value[0].get<float>(), value[1].get<float>(), value[2].get<float>(), value[3].get<float>());
                        }
                    }
                }
            }
            else
            {
                LOG_ERROR("Config file missing '{}' section", sectionName);
            }
        }
    };
}

// Global config instance
inline Config::AppConfig& config = Config::AppConfig::Get();
