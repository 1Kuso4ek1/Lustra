#pragma once
#include <AssetLoader.hpp>
#include <Multithreading.hpp>

#include <LLGL/Log.h>
#include <LLGL/Texture.h>

#include <filesystem>
#include <typeindex>

namespace dev
{

class AssetManager : public Singleton<AssetManager>
{
public:
    template<class T>
    std::shared_ptr<T> Load(const std::filesystem::path& path)
    {
        auto it = assets.find(path);

        if(it != assets.end())
        {
            auto asset = *(it->second);

            if(typeid(T) == typeid(asset))
            {
                LLGL::Log::Printf(
                    LLGL::Log::ColorFlags::Bold | LLGL::Log::ColorFlags::Green,
                    "Asset %s already loaded.\n",
                    path.string().c_str()
                );

                return std::static_pointer_cast<T>(it->second);
            }
        }

        auto& loader = loaders[std::type_index(typeid(T))];

        if(!loader)
        {
            LLGL::Log::Errorf(
                LLGL::Log::ColorFlags::StdError,
                "No loader found for asset type %s\n", typeid(T).name()
            );
            return nullptr;
        }

        auto asset = loader->Load(path);

        assets[path] = asset;

        return std::static_pointer_cast<T>(asset);
    }

    template<class T>
    void Unload(const std::filesystem::path& path)
    {
        assets.erase(path);
    }

    template<class AssetType, class LoaderType>
    void AddLoader()
    {
        loaders[typeid(AssetType)] = &LoaderType::Get();
    }

private:
    std::unordered_map<std::filesystem::path, std::shared_ptr<Asset>> assets;
    std::unordered_map<std::type_index, AssetLoader*> loaders;
};

}
