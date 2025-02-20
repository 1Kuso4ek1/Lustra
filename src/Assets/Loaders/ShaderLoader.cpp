#include <ShaderLoader.hpp>

namespace dev
{

AssetPtr VertexShaderLoader::Load(const std::filesystem::path& path)
{
    auto shader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, path);
    
    auto asset = std::make_shared<VertexShaderAsset>(shader);
    asset->path = path;
    asset->loaded = true;

    return asset;
}

AssetPtr FragmentShaderLoader::Load(const std::filesystem::path& path)
{
    auto shader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, path);

    auto asset = std::make_shared<FragmentShaderAsset>(shader);
    asset->path = path;
    asset->loaded = true;

    return asset;
}

}
