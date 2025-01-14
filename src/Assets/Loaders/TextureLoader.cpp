#include <TextureLoader.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace dev
{

AssetPtr TextureLoader::Load(const std::filesystem::path& path)
{
    if(!defaultTexture)
        LoadDefaultData();

    if(path.filename() == "default")
        return defaultTextureAsset; // Just the default texture

    auto textureAsset = std::make_shared<TextureAsset>(defaultTexture);
    textureAsset->sampler = anisotropySampler;

    LLGL::Log::Printf(
        LLGL::Log::ColorFlags::Bold | LLGL::Log::ColorFlags::Green,
        "Loading texture \"%s\"\n",
        path.string().c_str()
    );

    auto loadUint = [textureAsset, path]()
    {
        int width, height, channels;
        unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &channels, 4);

        if(data)
        {
            textureAsset->imageView.data = data;
            textureAsset->imageView.dataSize = width * height * 4 * sizeof(unsigned char);
            
            textureAsset->textureDesc.extent = { (uint32_t)width, (uint32_t)height, 1 };
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
        float* data = stbi_loadf(path.string().c_str(), &width, &height, &channels, 3);

        if(data)
        {
            textureAsset->imageView.data = data;
            textureAsset->imageView.dataSize = width * height * 3;
            textureAsset->imageView.dataType = LLGL::DataType::Float32;
            textureAsset->imageView.format = LLGL::ImageFormat::RGB;
            
            textureAsset->textureDesc.extent = { (uint32_t)width, (uint32_t)height, 1 };
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

            LLGL::OpenGL::ResourceNativeHandle nativeHandle;
            textureAsset->texture->GetNativeHandle(&nativeHandle, sizeof(nativeHandle));
            textureAsset->nativeHandle = nativeHandle.id;
          
            LLGL::Log::Printf(
                LLGL::Log::ColorFlags::Bold | LLGL::Log::ColorFlags::Green,
                "Texture \"%s\" loaded.\n",
                path.string().c_str()
            );
        }

        stbi_image_free((void*)textureAsset->imageView.data);
    };

    // Adapt hdr for the multithreaded loading
    if(path.extension() != ".hdr") // Add a "separateThread" parameter
    {
        Multithreading::Get().AddJob(loadUint);
        Multithreading::Get().AddMainThreadJob(create);
    }
    else
    {
        loadFloat();
        //loadUint();
        create();
    }

    return textureAsset;
}

void TextureLoader::LoadDefaultData()
{
    LLGL::SamplerDescriptor samplerDesc;
    samplerDesc.maxAnisotropy = 8;
    anisotropySampler = Renderer::Get().CreateSampler(samplerDesc);

    LLGL::TextureDescriptor textureDesc;
    textureDesc.extent = { 1, 1, 1 };
    
    unsigned char defaultData[] = { 255, 20, 147, 255 };

    LLGL::ImageView imageView;
    imageView.dataSize = 4;
    imageView.data = defaultData;
    imageView.format = LLGL::ImageFormat::RGBA;
    imageView.dataType = LLGL::DataType::UInt8;

    defaultTexture = Renderer::Get().CreateTexture(textureDesc, &imageView);

    defaultTextureAsset = std::make_shared<TextureAsset>(defaultTexture);
    defaultTextureAsset->sampler = anisotropySampler;

    LLGL::OpenGL::ResourceNativeHandle nativeHandle;
    defaultTextureAsset->texture->GetNativeHandle(&nativeHandle, sizeof(nativeHandle));
    defaultTextureAsset->nativeHandle = nativeHandle.id;
}

}
