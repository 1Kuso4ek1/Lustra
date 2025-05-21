#pragma once
#include <AssetLoader.hpp>
#include <ScriptAsset.hpp>

namespace lustra
{

class ScriptLoader final : public AssetLoader, public Singleton<ScriptLoader>
{
public:
    AssetPtr Load(
        const std::filesystem::path& path,
        AssetPtr existing = nullptr,
        bool async = true
    ) override;

};

}
