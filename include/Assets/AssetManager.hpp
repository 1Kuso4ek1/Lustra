#pragma once
#include <AssetLoader.hpp>
#include <Multithreading.hpp>

#include <LLGL/Log.h>
#include <LLGL/Texture.h>

#include <filesystem>
#include <typeindex>

namespace lustra
{

class AssetManager : public Singleton<AssetManager>
{
public:
    using AssetStorage = std::unordered_map<std::filesystem::path, std::pair<std::type_index, AssetPtr>>;

    ~AssetManager()
    {
        StopWatch();
    }

    template<class T>
    std::shared_ptr<T> Load(
        const std::filesystem::path& path,
        bool relativeToAssetsDir = false,
        bool useCache = true
    )
    {
        auto assetPath = GetAssetPath<T>(path, relativeToAssetsDir);

        if(useCache)
        {
            auto it = assets.find(assetPath);

            if(it != assets.end())
            {
                if(typeid(T) == it->second.first)
                    return std::static_pointer_cast<T>(it->second.second);
            }
        }

        auto loader = GetAssetLoader<T>();

        if(!loader)
            return nullptr;

        auto asset = loader->Load(assetPath);

        if(!asset)
            return nullptr;

        if(fsWatch)
            if(std::filesystem::exists(assetPath))
                timestamps[assetPath] = std::filesystem::last_write_time(assetPath);

        asset->path = assetPath;

        assets.emplace(assetPath, std::pair(std::type_index(typeid(T)), asset));

        return std::static_pointer_cast<T>(asset);
    }

    template<class T>
    auto GetAssetPath(const std::filesystem::path& path, bool relativeToAssetsDir) const
    {
        auto assetPath = path;

        if(relativeToAssetsDir)
        {
            auto relativeAssetPath = assetsRelativePaths.find(std::type_index(typeid(T)));

            if(relativeAssetPath != assetsRelativePaths.end())
                assetPath = assetsDirectory / relativeAssetPath->second / path;
            else
                assetPath = assetsDirectory / path;
        }

        return assetPath;
    }

    std::filesystem::path GetAssetsDirectory() const
    {
        return assetsDirectory;
    }

    AssetStorage& GetAssets()
    {
        return assets;
    }

    template<class T>
    void Write(std::shared_ptr<T> asset, const std::filesystem::path& path, bool relativeToAssetsDir = false)
    {
        auto assetPath = GetAssetPath<T>(path, relativeToAssetsDir);

        assets.emplace(assetPath, std::pair(std::type_index(typeid(T)), asset));

        auto loader = GetAssetLoader<T>();

        if(!loader)
            return;

        loader->Write(asset, assetPath);

        if(fsWatch)
            timestamps[assetPath] = std::filesystem::last_write_time(assetPath);

        asset->path = assetPath;
    }

    template<class T>
    void Write(std::shared_ptr<T> asset)
    {
        Write(asset, asset->path);
    }

    void SetAssetsDirectory(const std::filesystem::path& path)
    {
        assetsDirectory = path;
    }

    template<class T>
    void Unload(const std::filesystem::path& path)
    {
        assets.erase(path);

        if(fsWatch)
            timestamps.erase(path);
    }

    template<class AssetType, class LoaderType>
    void AddLoader(std::filesystem::path relativePath = "")
    {
        if(loaders.find(typeid(AssetType)) != loaders.end())
            return;

        loaders[typeid(AssetType)] = &LoaderType::Get();
        assetsRelativePaths[typeid(AssetType)] = relativePath;
    }

    template<class T>
    void RemoveLoader()
    {
        loaders[typeid(T)]->Reset();
        
        loaders.erase(typeid(T));
        assetsRelativePaths.erase(typeid(T));
    }

    void LaunchWatch()
    {
        fsWatch = true;

        watchFuture = std::async(std::launch::async, [&]()
        {
            while(fsWatch)
            {
                for(auto& i : timestamps)
                {
                    if(std::filesystem::last_write_time(i.first) != i.second)
                    {
                        LLGL::Log::Printf(
                            LLGL::Log::ColorFlags::Blue,
                            "File %s modified\n",
                            i.first.string().c_str()
                        );

                        auto& asset = assets.at(i.first);
                        auto loader = loaders[std::type_index(asset.first)];

                        std::this_thread::sleep_for(1s);

                        Multithreading::Get().AddJob({ nullptr, [&]()
                        {
                            loader->Load(i.first, asset.second);
                        } });

                        timestamps[i.first] = std::filesystem::last_write_time(i.first);
                    }
                }

                std::this_thread::sleep_for(0.3s);
            }
        });
    }

    void StopWatch()
    {
        fsWatch = false;

        watchFuture.wait();
    }

private:
    template<class T>
    AssetLoader* GetAssetLoader()
    {
        auto loader = loaders[std::type_index(typeid(T))];

        if(!loader)
        {
            LLGL::Log::Errorf(
                LLGL::Log::ColorFlags::StdError,
                "No loader found for asset type %s\n", typeid(T).name()
            );

            return nullptr;
        }

        return loader;
    }

private:
    bool fsWatch = false;

    std::future<void> watchFuture;

    std::filesystem::path assetsDirectory = "assets";

    AssetStorage assets;

    std::unordered_map<std::filesystem::path, std::filesystem::file_time_type> timestamps;
    
    std::unordered_map<std::type_index, std::filesystem::path> assetsRelativePaths;
    std::unordered_map<std::type_index, AssetLoader*> loaders;
};

}
