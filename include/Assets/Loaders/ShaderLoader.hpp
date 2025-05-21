#pragma once
#include <AssetLoader.hpp>
#include <ShaderAsset.hpp>

namespace lustra
{

class VertexShaderLoader final : public AssetLoader, public Singleton<VertexShaderLoader>
{
public:
    AssetPtr Load(
        const std::filesystem::path& path,
        AssetPtr existing = nullptr,
        bool async = true
    ) override;
};

class FragmentShaderLoader final : public AssetLoader, public Singleton<FragmentShaderLoader>
{
public:
    AssetPtr Load(
        const std::filesystem::path& path,
        AssetPtr existing = nullptr,
        bool async = true
    ) override;
};

}
