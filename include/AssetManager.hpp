#pragma once
#include <AssetHandle.hpp>
#include <Multithreading.hpp>

#include <LLGL/Log.h>
#include <LLGL/Texture.h>

#include <filesystem>

namespace dev
{

class AssetManager
{
public:
    static AssetManager& Get();

public:
    template<class T>
    AssetHandle<T> Load(const std::filesystem::path& path);

    template<class T>
    AssetHandle<T> Get(uint64_t id);

    template<class T>
    void Unload(uint64_t id);

private:
    AssetManager() {};

private:
    uint64_t idCounter = 0;

    std::unordered_map<uint64_t, AssetHandle<LLGL::Texture*>> textures;
};

}
