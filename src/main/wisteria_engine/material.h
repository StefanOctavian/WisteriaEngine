#pragma once

#include "core/gpu/shader.h"
#include "utils/glm_utils.h"
#include "texture.h"

namespace engine
{
    class Material
    {
    public:
        Material(): shader(nullptr) {}
        Material(Shader *shader);
        ~Material();

        void SetInt(std::string name, int value);
        void SetFloat(std::string name, float value);
        void SetIVec2(std::string name, glm::ivec2 value);
        void SetVec2(std::string name, glm::vec2 value);
        void SetVec3(std::string name, glm::vec3 value);
        void SetVec4(std::string name, glm::vec4 value);
        void SetMat3(std::string name, glm::mat3 value);
        void SetMat4(std::string name, glm::mat4 value);
        void SetTexture(std::string name, Texture *texture);

        Texture *GetTexture();
        void SetTexture(Texture *texture);

        void Use();

        glm::vec3 diffuseLight = glm::vec3(0.05f, 0.05f, 0.05f);
        glm::vec3 specularLight = glm::vec3(0.02f, 0.02f, 0.02f);
        float shininess = 0.5f;

        Shader *shader;
        bool wireframe = false;
        Texture *texture = nullptr;
        int instances = 1;

        bool blending = false;
        GLint blendFunction = GL_FUNC_ADD;
        GLint blendSrc = GL_SRC_ALPHA;
        GLint blendDst = GL_ONE_MINUS_SRC_ALPHA;

    private:
        enum UniformType
        {
            INT, FLOAT,
            IVEC2, 
            VEC2, VEC3, VEC4,
            MAT3, MAT4,
            TEXTURE
        };
        union UniformValue
        {
            int intValue;
            float floatValue;
            glm::ivec2 ivec2Value;
            glm::vec2 vec2Value; glm::vec3 vec3Value; glm::vec4 vec4Value;
            glm::mat3 mat3Value; glm::mat4 mat4Value;
            Texture *textureValue;
        };
        std::unordered_map<std::string, std::pair<UniformType, UniformValue>> uniforms;

        unsigned int numTextures = 0;
    };
}