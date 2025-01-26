#include <ModelLoader.hpp>
#include <Multithreading.hpp>
#include <EventManager.hpp>

namespace dev
{

AssetPtr ModelLoader::Load(const std::filesystem::path& path)
{
    if(!plane)
        LoadDefaultData();

    LLGL::Log::Printf(
        LLGL::Log::ColorFlags::Bold | LLGL::Log::ColorFlags::Green,
        "Loading model \"%s\"\n",
        path.string().c_str()
    );

    if(path.filename() == "plane")
        return std::make_shared<ModelAsset>(ModelAsset({ plane }));
    if(path.filename() == "cube")
        return std::make_shared<ModelAsset>(ModelAsset({ cube }));

    auto modelAsset = std::make_shared<ModelAsset>(ModelAsset({ cube }));

    auto load = [path, modelAsset, this]()
    {
        ImportModel(path, modelAsset);
    };

    auto create = [modelAsset]()
    {
        modelAsset->meshes = modelAsset->temporaryMeshes;
        modelAsset->temporaryMeshes.clear();

        for(auto& mesh : modelAsset->meshes)
            mesh->SetupBuffers();

        modelAsset->loaded = true;

        EventManager::Get().Dispatch(std::make_unique<AssetLoadedEvent>(modelAsset));
    };

    if(true) // Add a "separateThread" parameter
    {
        Multithreading::Get().AddJob({ load, create });
    }
    else
    {
        load();
        create();
    }
   
    return modelAsset;
}

void ModelLoader::LoadDefaultData()
{
    (plane = std::make_shared<Mesh>())->CreatePlane();
    (cube = std::make_shared<Mesh>())->CreateCube();
}

void ModelLoader::ImportModel(const std::filesystem::path& path, ModelAssetPtr modelAsset)
{
    auto flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes | aiProcess_LimitBoneWeights;

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 1);

    auto scene = importer.ReadFile(path.string(), flags);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        LLGL::Log::Errorf(
            LLGL::Log::ColorFlags::StdError,
            "Failed to load model: %s\n", importer.GetErrorString()
        );

        return;
    }

    modelAsset->meshes.reserve(scene->mNumMeshes);
    modelAsset->temporaryMeshes.reserve(scene->mNumMeshes);

    ProcessNode(scene->mRootNode, scene, modelAsset);

    LLGL::Log::Printf(
        LLGL::Log::ColorFlags::Bold | LLGL::Log::ColorFlags::Green,
        "Model \"%s\" loaded.\n",
        path.string().c_str()
    );
}

void ModelLoader::ProcessNode(aiNode* node, const aiScene* scene, std::shared_ptr<ModelAsset> modelAsset)
{
    for(uint32_t i = 0; i < node->mNumMeshes; i++)
        modelAsset->temporaryMeshes.push_back(ProcessMesh(scene->mMeshes[node->mMeshes[i]], scene));

    for(unsigned int i = 0; i < scene->mNumMaterials; i++)
        ProcessMaterial(scene->mMaterials[i], modelAsset);

    for(uint32_t i = 0; i < node->mNumChildren; i++)
        ProcessNode(node->mChildren[i], scene, modelAsset);
}

void ModelLoader::ProcessMaterial(aiMaterial* material, std::shared_ptr<ModelAsset> modelAsset)
{
    // TODO ...
}

MeshPtr ModelLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        
        vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

        if(mesh->mTextureCoords[0])
            vertex.coords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        else
            vertex.coords = { 0.0f, 0.0f };

        vertices.push_back(vertex);
    }

    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];

        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    
    return std::make_shared<Mesh>(vertices, indices, false);
}

}
