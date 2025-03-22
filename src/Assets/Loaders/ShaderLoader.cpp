#include <ShaderLoader.hpp>
#include <EventManager.hpp>

namespace lustra
{

AssetPtr VertexShaderLoader::Load(
    const std::filesystem::path& path,
    AssetPtr existing,
    bool async
)
{
    auto shader = Renderer::Get().CreateShader(LLGL::ShaderType::Vertex, path);
    
    auto asset = existing
        ? std::static_pointer_cast<VertexShaderAsset>(existing)
        : std::make_shared<VertexShaderAsset>(shader);

    if(existing)
    {
        Renderer::Get().Release(asset->shader);
        asset->shader = shader;
    }

    asset->loaded = true;

    EventManager::Get().Dispatch(std::make_unique<AssetLoadedEvent>(asset));

    return asset;
}

AssetPtr FragmentShaderLoader::Load(
    const std::filesystem::path& path,
    AssetPtr existing,
    bool async
)
{
    auto shader = Renderer::Get().CreateShader(LLGL::ShaderType::Fragment, path);

    auto asset = existing
        ? std::static_pointer_cast<FragmentShaderAsset>(existing)
        : std::make_shared<FragmentShaderAsset>(shader);

    if(existing)
    {
        Renderer::Get().Release(asset->shader);
        asset->shader = shader;
    }

    asset->loaded = true;

    EventManager::Get().Dispatch(std::make_unique<AssetLoadedEvent>(asset));

    return asset;
}

}
