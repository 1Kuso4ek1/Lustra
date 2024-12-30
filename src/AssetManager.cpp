#include <AssetManager.hpp>

namespace dev
{

template<class T>
AssetHandle<T> AssetManager::Load(const std::filesystem::path& path)
{
    LLGL::Log::Printf(LLGL::Log::ColorFlags::Bold | LLGL::Log::ColorFlags::Green, 
                      "Loading asset %s\n", path.string().c_str());

    
}

}
