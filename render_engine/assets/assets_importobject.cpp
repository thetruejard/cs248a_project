#include "assets/assets.h"
#include "utils/assimputils.h"
#include "graphics/mesh.h"
#include "objects/go_mesh.h"

#include "assimp/importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include <iostream>
#include <unordered_map>
#include <map>
#include <filesystem>



class importObject_Context {
public:

    RenderEngine* engine;
    std::filesystem::path directory;

    const aiScene* scene;

    // Same size as this->scene->mMeshes.
    std::vector<Ref<Mesh>> meshes;

    // Same size as this->scene->mMaterials.
    std::vector<Ref<Material>> materials;

    // Same size as this->scene->mTextures.
    std::vector<Ref<Texture>> embeddedTextures;
    
    // External (non-embedded) textures. NOT included in this->scene->mTextures.
    // TODO: Change to allow semantic comparison/symlinks
    std::map<std::filesystem::path,Ref<Texture>> externalTextures;

    // A mapping from node names to light objects.
    std::unordered_map<std::string, aiLight*> lights;

};


static void processLight(GO_Light* light, aiLight* in_light) {
    switch (in_light->mType) {
    case aiLightSource_DIRECTIONAL:
        light->type = GO_Light::Type::Directional;
        light->direction = glm::normalize(AssimpUtils::toVec3(in_light->mDirection));
        light->color = AssimpUtils::toVec3(in_light->mColorDiffuse);
        std::cout << "Directional light: {color:[" << light->color.r << "," << light->color.g << "," <<
            light->color.b << "], direction: [" << light->direction.x << "," <<
            light->direction.y << "," << light->attenuation.z << "]}\n";
        break;
    case aiLightSource_POINT:
        light->type = GO_Light::Type::Point;
        light->attenuation = glm::vec3(
            in_light->mAttenuationConstant,
            in_light->mAttenuationLinear,
            in_light->mAttenuationQuadratic
        );
        light->color = AssimpUtils::toVec3(in_light->mColorDiffuse);
        std::cout << "Point light: {color:[" << light->color.r << "," << light->color.g << "," <<
            light->color.b << "], attenuation: [" << light->attenuation.x << "," <<
            light->attenuation.y << "," << light->attenuation.z << "]}\n";
        break;
    case 0://aiLightSource_SPOT:
        light->type = GO_Light::Type::Spot;
        light->direction = glm::normalize(AssimpUtils::toVec3(in_light->mDirection));
        light->innerOuterAngles = glm::vec2(
            in_light->mAngleInnerCone,
            in_light->mAngleInnerCone
        );
        light->attenuation = glm::vec3(
            in_light->mAttenuationConstant,
            in_light->mAttenuationLinear,
            in_light->mAttenuationQuadratic
        );
        light->color = AssimpUtils::toVec3(in_light->mColorDiffuse);
        break;
    // TODO: Consider supporting area lights.
    default:
        light->type = GO_Light::Type::Disabled;
        break;
    }
}


static Ref<Texture> processTexture(importObject_Context& context, std::string texturePath) {
    auto texAndIdx = context.scene->GetEmbeddedTextureAndIndex(texturePath.c_str());
    if (texAndIdx.first) {
        std::cout << "Assimp found an embedded texture: " << texturePath << "\n";
        return Ref<Texture>();
    }
    else {
        // Load from a file.
        std::string::size_type n = 0;
        while ((n = texturePath.find("%20", n)) != std::string::npos)
        {
            texturePath.replace(n, 3, " ");
            n += 1;
        }
        std::filesystem::path fullPath = context.directory / texturePath;
        auto result = context.externalTextures.find(fullPath);
        if (result != context.externalTextures.end()) {
            std::cout << "Reusing " << fullPath << "\n";
            return result->second;
        }
        std::cout << "Loading " << fullPath << "\n";
        Ref<Texture> tex = Assets::importTexture(*context.engine, fullPath);
        context.externalTextures[fullPath] = tex;
        return tex;
    }
}


static Ref<Material> processMaterial(importObject_Context& context, int materialIdx) {
    Ref<Material> material = context.materials[materialIdx];
    if (material) {
        return material;
    }
    material = context.engine->createMaterial();

    aiMaterial* in_material = context.scene->mMaterials[materialIdx];
    aiString tempStr;
    aiColor3D tempColor3;
    float tempFloat;

    in_material->Get(AI_MATKEY_NAME, tempStr);
    material->setName(AssimpUtils::toStr(tempStr));

    // Diffuse color.
    if (in_material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        in_material->GetTexture(aiTextureType_DIFFUSE, 0, &tempStr);
        Ref<Texture> diffuseTexture = processTexture(context, AssimpUtils::toStr(tempStr));
        material->assignDiffuseTexture(diffuseTexture);
    }
    in_material->Get(AI_MATKEY_COLOR_DIFFUSE, tempColor3);
    material->assignDiffuseColor(glm::vec4(AssimpUtils::toVec3(tempColor3),
        material->getDiffuseTexture() ? 0.0f : 1.0f
    ));

    // Metalness.
    if (in_material->GetTextureCount(aiTextureType_METALNESS) > 0) {
        in_material->GetTexture(aiTextureType_METALNESS, 0, &tempStr);
        Ref<Texture> metalnessTexture = processTexture(context, AssimpUtils::toStr(tempStr));
        material->assignMetalnessTexture(metalnessTexture);
    }
    in_material->Get(AI_MATKEY_METALLIC_FACTOR, tempFloat);
    material->assignMetalness(tempFloat);

    // Roughness.
    if (in_material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0) {
        in_material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &tempStr);
        Ref<Texture> roughnessTexture = processTexture(context, AssimpUtils::toStr(tempStr));
        material->assignRoughnessTexture(roughnessTexture);
    }
    in_material->Get(AI_MATKEY_ROUGHNESS_FACTOR, tempFloat);
    material->assignRoughness(tempFloat);

    // Normal.
    if (in_material->GetTextureCount(aiTextureType_NORMALS) > 0) {
        in_material->GetTexture(aiTextureType_NORMALS, 0, &tempStr);
        Ref<Texture> normalTexture = processTexture(context, AssimpUtils::toStr(tempStr));
        material->assignNormalTexture(normalTexture);
    }

    return material;
}


// TODO: Only supports triangles (vertex count of 3 is hardcoded).
// TODO: Only supports one set of texture coordinates. See Vertex and Mesh.
static Ref<GO_Mesh> processMesh(importObject_Context& context, int meshIdx) {
    Ref<GO_Mesh> obj = context.engine->createObject<GO_Mesh>();
    if (!obj) {
        return nullptr;
    }

    aiMesh* in_mesh = context.scene->mMeshes[meshIdx];

    std::string _name = AssimpUtils::toStr(in_mesh->mName);
    if (!_name.empty()) {
        obj->setName(_name + "." + std::to_string(obj->getID()));
    }

    Ref<Mesh> mesh = context.meshes[meshIdx];
    if (!mesh) {
        mesh = context.engine->createMesh();
        context.meshes[meshIdx] = mesh;
        obj->assignMesh(mesh);
    }
    else {
        obj->assignMesh(mesh);
        // The mesh has already been processed.
        return obj;
    }


    Vertex* vertBuffer = mesh->createVertexBuffer(in_mesh->mNumVertices);
    // TODO: 3 IS HARDCODED AS THE NUMBER OF VERTICES PER FACE.
    VertexIndex* idxBuffer = mesh->createIndexBuffer(in_mesh->mNumFaces * (size_t)3);

    // Vertices.
    for (unsigned int i = 0; i < in_mesh->mNumVertices; i++) {
        Vertex& vertex = vertBuffer[i];

        vertex.position = AssimpUtils::toVec3(in_mesh->mVertices[i]);
        // Normals always exist with the GenNormals assimp process specified.
        vertex.normal = AssimpUtils::toVec3(in_mesh->mNormals[i]);
    
        // Texture Coordinates/Tangents/Bitangents.
        // Up to 8 UV maps may exist. We only take the first one.
        if (in_mesh->mTextureCoords[0]) {
            vertex.uv = AssimpUtils::toVec2(in_mesh->mTextureCoords[0][i]);
            vertex.tangent = AssimpUtils::toVec3(in_mesh->mTangents[i]);
            vertex.bitangent = AssimpUtils::toVec3(in_mesh->mBitangents[i]);
        }
        else {
            vertex.uv = glm::vec2(0.0f);
            vertex.tangent = glm::vec3(0.0f);
            vertex.bitangent = glm::vec3(0.0f);
        }
    }
    
    // Faces (indices).
    for (unsigned int i = 0; i < in_mesh->mNumFaces; i++)
    {
        aiFace face = in_mesh->mFaces[i];
        // TODO: Only supports triangles.
        // TODO: Consider SIMD copying?
        assert(face.mNumIndices == 3);
        for (unsigned int j = 0; j < 3; j++) {
            idxBuffer[i * 3 + j] = face.mIndices[j];
        }
    }

    // Material.
    if (in_mesh->mMaterialIndex >= 0) {
        Ref<Material> material = processMaterial(context, in_mesh->mMaterialIndex);
        mesh->assignMaterial(material);
    }

    mesh->uploadMesh();

    return obj;
}


static Ref<GameObject> processNode(importObject_Context& context, aiNode* node) {
    // Select object type based on node associations (to lights/bones/etc).
    std::string name = AssimpUtils::toStr(node->mName);
    Ref<GameObject> obj = nullptr;

    if (!name.empty()) {
        // Check lights.
        auto _l = context.lights.find(name);
        if (_l != context.lights.end()) {
            obj = context.engine->createObject<GO_Light>();
            if (!obj) {
                return nullptr;
            }
            processLight((GO_Light*)obj.get(), _l->second);
        }
    }
    if (obj == nullptr) {
        obj = context.engine->createObject<GameObject>();
        if (obj == nullptr) {
            return nullptr;
        }
    }

    if (!name.empty()) {
        obj->setName(name);
    }
    obj->setLocalMatrix(AssimpUtils::toMat(node->mTransformation));

    // Process all meshes at this node, if any.
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        // The node object only contains indices to the objects in the scene. 
        // The scene contains all the data, the nodes just store parent/child relations.
        aiMesh* mesh = context.scene->mMeshes[node->mMeshes[i]];
        // TODO: For now, supports triangles only. Consider supporting vertices and lines?
        if (mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE) {
            processMesh(context, node->mMeshes[i])->setParent(obj, false);
        }
    }
    // Recursively process child nodes.
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(context, node->mChildren[i])->setParent(obj, false);
    }
    return obj;
}


Ref<GameObject> Assets::importObject(RenderEngine& engine, std::filesystem::path path) {

    importObject_Context context;
    context.engine = &engine;
    context.directory = std::filesystem::absolute(path.parent_path());

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path.string(),
        aiProcess_Triangulate |
        aiProcess_SortByPType |
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace |
        // aiProcess_OptimizeGraph |    // TODO: Test reasonable use cases.
        aiProcess_OptimizeMeshes |
        aiProcess_GenUVCoords |
        aiProcess_TransformUVCoords |
        NULL);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        // TODO: Error handling.
        return nullptr;
    }

    context.scene = scene;

    context.meshes.resize(scene->mNumMeshes);
    context.materials.resize(scene->mNumMaterials);
    context.embeddedTextures.resize(scene->mNumTextures);

    for (unsigned int i = 0; i < scene->mNumLights; i++) {
        aiLight* light = scene->mLights[i];
        std::string name = AssimpUtils::toStr(light->mName);
        context.lights[name] = light;
    }

    return processNode(context, scene->mRootNode);
}
