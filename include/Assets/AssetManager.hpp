#pragma once
#include <AssetLoader.hpp>
#include <Multithreading.hpp>

#include <LLGL/Log.h>

#include <filesystem>
#include <typeindex>

namespace lustra
{

class AssetManager final : public Singleton<AssetManager>
{
public:
    using AssetStorage = std::unordered_map<std::filesystem::path, std::pair<std::type_index, AssetPtr>>;

    ~AssetManager() override
    {
        StopWatch();
    }

    template<class T>
    std::shared_ptr<T> Load(
        const std::filesystem::path& path,
        const bool relativeToAssetsDir = false,
        const bool useCache = true,
        const bool async = true
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

        // try-catch?
        auto asset = loader->Load(assetPath, nullptr, async);

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
    auto GetAssetPath(const std::filesystem::path& path, const bool relativeToAssetsDir) const
    {
        auto assetPath = path;

        if(relativeToAssetsDir)
        {
            const auto relativeAssetPath = assetsRelativePaths.find(std::type_index(typeid(T)));

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

    void Unload(const std::filesystem::path& path)
    {
        const auto it = assets.find(path);

        if(it == assets.end())
            return;

        if(const auto loader = loaders[it->second.first])
            loader->Unload(it->second.second);

        assets.erase(it);

        if(fsWatch)
            timestamps.erase(path);
    }

    template<class T>
    void Write(std::shared_ptr<T> asset, const std::filesystem::path& path, const bool relativeToAssetsDir = false)
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

    template<class AssetType, class LoaderType>
    void AddLoader(const std::filesystem::path relativePath = "")
    {
        if(loaders.contains(typeid(AssetType)))
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
        if(fsWatch)
            return;

        fsWatch = true;

        watchFuture = std::async(std::launch::async, [&]()
        {
            while(fsWatch)
            {
                for(auto& [path, time] : timestamps)
                {
                    if(std::filesystem::last_write_time(path) > time)
                    {
                        LLGL::Log::Printf(
                            LLGL::Log::ColorFlags::Blue,
                            "File %s modified\n",
                            path.string().c_str()
                        );

                        auto& [type, asset] = assets.at(path);
                        auto loader = loaders[std::type_index(type)];

                        std::this_thread::sleep_for(1s);

                        Multithreading::Get().AddJob({ nullptr, [&]
                        {
                            loader->Load(path, asset);
                        } });

                        timestamps[path] = std::filesystem::last_write_time(path);
                    }
                }

                std::this_thread::sleep_for(0.3s);
            }
        });
    }

    void StopWatch()
    {
        fsWatch = false;

        if(watchFuture.valid())
            watchFuture.wait();
    }

private:
    template<class T>
    AssetLoader* GetAssetLoader()
    {
        const auto loader = loaders[std::type_index(typeid(T))];

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
