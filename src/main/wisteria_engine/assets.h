#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include "core/gpu/mesh.h"
#include "core/gpu/shader.h"
#include "stb/stb_image.h"
#include "meshplusplus.h"
#include "material.h"
#include "texture.h"

// to whoever wrote gfxc framework:
// seriously, did you never learn to add parantheses around macro definitions?
#pragma warning(disable: 4005)  // disable macro redefinition warning
#define PATH_JOIN(...) (text_utils::Join(std::vector<std::string>{__VA_ARGS__}, std::string(1, PATH_SEPARATOR)))

namespace engine
{
    class Assets
    {
    public:
        static void LoadMesh(const std::string &name, const std::string &fileLocation, const std::string &fileName)
        {
            MeshPlusPlus *mesh = new MeshPlusPlus(name);
            mesh->LoadMesh(PATH_JOIN(lookupDirectory, fileLocation.c_str()), fileName.c_str());
            meshes[name] = mesh;
        }

        static void AddPath(const std::string &name, const std::string &path)
        {
            paths[name] = PATH_JOIN(lookupDirectory, path.c_str());
        }

        static Shader *CreateShader(const std::string &vertexShader, const std::string &fragmentShader)
        {
            Shader *shader = new Shader("shader");
            shader->AddShader(paths[vertexShader], GL_VERTEX_SHADER);
            shader->AddShader(paths[fragmentShader], GL_FRAGMENT_SHADER);
            shader->CreateAndLink();
            return shader;
        }

        static void LoadShader(const std::string &name, const std::string &vertexShader, 
                               const std::string &fragmentShader)
        {
            Shader *shader = CreateShader(vertexShader, fragmentShader);
            shaders[name] = shader;
        }

        static Shader *CreateShader(const std::string &vertexShader, const std::string &geometryShader,
                                    const std::string &fragmentShader)
        {
            std::string vertexPath = paths[vertexShader];
            std::string geometryPath = paths[geometryShader];
            std::string fragmentPath = paths[fragmentShader];

            if (vertexPath.empty()) std::cerr << "Vertex shader path not found: " << vertexShader << std::endl;
            if (geometryPath.empty()) std::cerr << "Geometry shader path not found: " << geometryShader << std::endl;
            if (fragmentPath.empty()) std::cerr << "Fragment shader path not found: " << fragmentShader << std::endl;
            if (vertexPath.empty() || geometryPath.empty() || fragmentPath.empty()) exit(1);

            Shader *shader = new Shader("shader");
            shader->AddShader(paths[vertexShader], GL_VERTEX_SHADER);
            shader->AddShader(paths[geometryShader], GL_GEOMETRY_SHADER);
            shader->AddShader(paths[fragmentShader], GL_FRAGMENT_SHADER);
            shader->CreateAndLink();
            return shader;
        }

        static void LoadShader(const std::string &name, const std::string &vertexShader,
                               const std::string &geometryShader, const std::string &fragmentShader)
        {
            Shader *shader = CreateShader(vertexShader, geometryShader, fragmentShader);
            shaders[name] = shader;
        }

        static void CreateMaterial(const std::string &name, const std::string &shaderName)
        {
            Shader *shader = shaders[shaderName];
            if (!shader) {
                std::cerr << "Shader " << shaderName << " not found";
                exit(1);
            }
            materials[name] = Material(shader);
        }

        static void LoadTexture2D(const std::string &name, const std::string &fileLocation, 
                                  const std::string &fileName)
        {
            Texture *texture = Texture2D::Load(PATH_JOIN(lookupDirectory, 
                                                         fileLocation.c_str(), fileName)
                                                    .c_str());
            textures[name] = texture;
        }

        static void LoadCubemapTexture(const std::string &name, const std::string &fileLocation, 
                                       const std::string &pos_x, const std::string &pos_y, 
                                       const std::string &pos_z, const std::string &neg_x, 
                                       const std::string &neg_y, const std::string &neg_z)
        {
            Cubemap *texture = Cubemap::Load(
                PATH_JOIN(lookupDirectory, fileLocation.c_str(), pos_x).c_str(),
                PATH_JOIN(lookupDirectory, fileLocation.c_str(), pos_y).c_str(),
                PATH_JOIN(lookupDirectory, fileLocation.c_str(), pos_z).c_str(),
                PATH_JOIN(lookupDirectory, fileLocation.c_str(), neg_x).c_str(),
                PATH_JOIN(lookupDirectory, fileLocation.c_str(), neg_y).c_str(),
                PATH_JOIN(lookupDirectory, fileLocation.c_str(), neg_z).c_str()
            );

            textures[name] = texture;
        }
        
        static std::string lookupDirectory;
        static std::unordered_map<std::string, Mesh *> meshes;
        static std::unordered_map<std::string, std::string> paths;
        static std::unordered_map<std::string, Shader *> shaders;
        static std::unordered_map<std::string, Material> materials;
        static std::unordered_map<std::string, Texture *> textures;
    };
}