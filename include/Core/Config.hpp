#pragma once
#include <LLGL/Types.h>

#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>

#include <filesystem>
#include <fstream>

namespace LLGL
{

template<class Archive>
void save(Archive& archive, const Extent2D& extent)
{
    archive(
        cereal::make_nvp("width", extent.width),
        cereal::make_nvp("height", extent.height)
    );
}

template<class Archive>
void load(Archive& archive, Extent2D& extent)
{
    archive(extent.width, extent.height);
}

}

namespace lustra
{

struct Config
{
    LLGL::Extent2D resolution{ 1280, 720 };

    bool vsync = false;
    bool fullscreen = false;

    std::string title = "Application";
    std::string assetsRoot = "assets";
    std::string mainScene = "main.scn";
    std::string imGuiFontPath;
    std::string imGuiLayoutPath;

    std::filesystem::path configPath;

    void Save(const std::filesystem::path& path) const
    {
        std::ofstream file(path);

        if(file.is_open())
        {
            cereal::JSONOutputArchive archive(file);
            archive(cereal::make_nvp("config", *this));
        }
    }

    static Config Load(const std::filesystem::path& path)
    {
        Config config;
        config.configPath = path;

        std::ifstream file(path);

        if(file.is_open())
        {
            cereal::JSONInputArchive archive(file);
            archive(config);
        }

        return config;
    }

    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            CEREAL_NVP(resolution),
            CEREAL_NVP(vsync),
            CEREAL_NVP(fullscreen),
            CEREAL_NVP(title),
            CEREAL_NVP(assetsRoot),
            CEREAL_NVP(mainScene),
            CEREAL_NVP(imGuiFontPath),
            CEREAL_NVP(imGuiLayoutPath)
        );
    }
};

}
