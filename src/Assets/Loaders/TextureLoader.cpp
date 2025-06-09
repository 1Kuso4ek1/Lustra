#include <TextureLoader.hpp>
#include <EventManager.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace lustra
{

AssetPtr TextureLoader::Load(
    const std::filesystem::path& path,
    const AssetPtr existing,
    const bool async
)
{
    if(!defaultTexture)
        LoadDefaultData();

    if(path.filename() == "default")
        return defaultTextureAsset; // Just the default texture
    else if(path.filename() == "empty")
        return emptyTextureAsset; // 0 0 0 255

    auto textureAsset = existing
        ? std::static_pointer_cast<TextureAsset>(existing)
        : std::make_shared<TextureAsset>(defaultTexture);

    textureAsset->sampler = anisotropySampler;

    LLGL::Log::Printf(
        LLGL::Log::ColorFlags::Bold | LLGL::Log::ColorFlags::Green,
        "Loading texture \"%s\"\n",
        path.string().c_str()
    );

    auto loadUint = [textureAsset, path]()
    {
        int width, height, channels;

        if(const auto data = stbi_load(path.string().c_str(), &width, &height, &channels, 4))
        {
            textureAsset->imageView.data = data;
            textureAsset->imageView.dataSize = width * height * 4 * sizeof(unsigned char);

            textureAsset->textureDesc.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
        }
        else
            LLGL::Log::Errorf(
                LLGL::Log::ColorFlags::StdError,
                "Failed to load texture: %s\n", path.string().c_str()
            );
    };

    auto loadFloat = [textureAsset, path]()
    {
        int width, height, channels;

        if(const auto data = stbi_loadf(path.string().c_str(), &width, &height, &channels, 3))
        {
            textureAsset->imageView.data = data;
            textureAsset->imageView.dataSize = width * height * 3;
            textureAsset->imageView.dataType = LLGL::DataType::Float32;
            textureAsset->imageView.format = LLGL::ImageFormat::RGB;

            textureAsset->textureDesc.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
            textureAsset->textureDesc.format = LLGL::Format::RGB16Float;
        }
        else
            LLGL::Log::Errorf(
                LLGL::Log::ColorFlags::StdError,
                "Failed to load texture: %s\n", path.string().c_str()
            );
    };

    auto create = [textureAsset, path]()
    {
        if(textureAsset->imageView.data)
        {
            textureAsset->texture = Renderer::Get().CreateTexture(textureAsset->textureDesc, &textureAsset->imageView);

            LLGL::OpenGL::ResourceNativeHandle nativeHandle{};
            textureAsset->texture->GetNativeHandle(&nativeHandle, sizeof(nativeHandle));
            textureAsset->nativeHandle = nativeHandle.id;

            LLGL::Log::Printf(
                LLGL::Log::ColorFlags::Bold | LLGL::Log::ColorFlags::Green,
                "Texture \"%s\" loaded.\n",
                path.string().c_str()
            );

            textureAsset->loaded = true;

            EventManager::Get().Dispatch(std::make_unique<AssetLoadedEvent>(textureAsset));
        }

        stbi_image_free(const_cast<void*>(textureAsset->imageView.data));
    };

    if(async)
        Multithreading::Get().AddJob({ path.extension() == ".hdr" ? std::function<void()>(loadFloat) : loadUint, create });
    else
    {
        if(path.extension() == ".hdr")
            loadFloat();
        else
            loadUint();

        create();
    }

    return textureAsset;
}

void TextureLoader::Unload(const AssetPtr& asset)
{
    LLGL::Log::Printf(
        LLGL::Log::ColorFlags::StdWarning,
        "Unloading texture \"%s\"\n",
        asset->path.string().c_str()
    );

    const auto textureAsset = std::static_pointer_cast<TextureAsset>(asset);

    if(textureAsset->texture)
        Renderer::Get().Release(textureAsset->texture);
}

void TextureLoader::Reset()
{
    if(defaultTexture)
    {
        Renderer::Get().Release(defaultTexture);
        Renderer::Get().Release(emptyTexture);
        Renderer::Get().Release(anisotropySampler);

        defaultTexture = nullptr;
        emptyTexture = nullptr;
        anisotropySampler = nullptr;
    }
}

void TextureLoader::LoadDefaultData()
{
    anisotropySampler = Renderer::Get().CreateSampler({ .maxAnisotropy = 16 });

    constexpr unsigned char defaultData[] = { 255, 20, 147, 255 };
    constexpr unsigned char emptyData[] = { 0, 0, 0, 255 };

    LLGL::ImageView imageView;
    imageView.dataSize = 4;
    imageView.data = defaultData;
    imageView.format = LLGL::ImageFormat::RGBA;
    imageView.dataType = LLGL::DataType::UInt8;

    defaultTexture = Renderer::Get().CreateTexture({ .extent = { 1, 1, 1 } }, &imageView);

    defaultTextureAsset = std::make_shared<TextureAsset>(defaultTexture);
    defaultTextureAsset->sampler = anisotropySampler;

    LLGL::OpenGL::ResourceNativeHandle nativeHandle{};
    defaultTextureAsset->texture->GetNativeHandle(&nativeHandle, sizeof(nativeHandle));
    defaultTextureAsset->nativeHandle = nativeHandle.id;

    defaultTextureAsset->loaded = true;

    imageView.data = emptyData;

    emptyTexture = Renderer::Get().CreateTexture({ .extent = { 1, 1, 1 } }, &imageView);

    emptyTextureAsset = std::make_shared<TextureAsset>(emptyTexture);
    emptyTextureAsset->sampler = anisotropySampler;

    emptyTextureAsset->texture->GetNativeHandle(&nativeHandle, sizeof(nativeHandle));
    emptyTextureAsset->nativeHandle = nativeHandle.id;

    emptyTextureAsset->loaded = true;
}

}
