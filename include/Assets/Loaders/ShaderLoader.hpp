#pragma once
#include <AssetLoader.hpp>
#include <ShaderAsset.hpp>

namespace lustra
{

class VertexShaderLoader : public AssetLoader, public Singleton<VertexShaderLoader>
{
public:
    AssetPtr Load(const std::filesystem::path& path) override;
};

class FragmentShaderLoader : public AssetLoader, public Singleton<FragmentShaderLoader>
{
public:
    AssetPtr Load(const std::filesystem::path& path) override;
};

}
