#include <iostream>
#include "material.h"
#include "light.h"

using namespace engine;

Material::Material(Shader *shader): shader(shader) {}

Material::~Material() {}

void Material::SetInt(std::string name, int value)
{
    UniformValue val;
    val.intValue = value;
    uniforms[name] = std::make_pair(INT, val);
}

void Material::SetFloat(std::string name, float value)
{
    UniformValue val;
    val.floatValue = value;
    uniforms[name] = std::make_pair(FLOAT, val);
}

void Material::SetIVec2(std::string name, glm::ivec2 value)
{
    UniformValue val;
    val.ivec2Value = value;
    uniforms[name] = std::make_pair(IVEC2, val);
}

void Material::SetVec2(std::string name, glm::vec2 value)
{
    UniformValue val;
    val.vec2Value = value;
    uniforms[name] = std::make_pair(VEC2, val);
}

void Material::SetVec3(std::string name, glm::vec3 value)
{
    UniformValue val;
    val.vec3Value = value;
    uniforms[name] = std::make_pair(VEC3, val);
}

void Material::SetVec4(std::string name, glm::vec4 value)
{
    UniformValue val;
    val.vec4Value = value;
    uniforms[name] = std::make_pair(VEC4, val);
}

void Material::SetMat3(std::string name, glm::mat3 value)
{
    UniformValue val;
    val.mat3Value = value;
    uniforms[name] = std::make_pair(MAT3, val);
}

void Material::SetMat4(std::string name, glm::mat4 value)
{
    UniformValue val;
    val.mat4Value = value;
    uniforms[name] = std::make_pair(MAT4, val);
}

void Material::SetTexture(std::string name, Texture *texture)
{
    if (uniforms.find(name) == uniforms.end()) {
        if (numTextures == MAX_2D_TEXTURES) {
            std::cout << "Maximum number of textures reached" << std::endl;
            std::abort();
        }
        numTextures += 1;
    }

    UniformValue val;
    val.textureValue = texture;
    uniforms[name] = std::make_pair(TEXTURE, val);
}

Texture *Material::GetTexture()
{
    return texture;
}

void Material::SetTexture(Texture *texture)
{
    if (!this->texture && texture && numTextures == MAX_2D_TEXTURES) {
        throw std::runtime_error("Maximum number of textures reached");
        return;
    }
    numTextures += (!this->texture) - (!texture);
    this->texture = texture;
}

void Material::Use()
{
    if (!shader || !shader->program)
        return;

    if (wireframe) {
        glLineWidth(1);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (blending) {
        glEnable(GL_BLEND);
        glBlendEquation(blendFunction);
        glBlendFunc(blendSrc, blendDst);
        glEnable(GL_DEPTH_TEST);
    } else {
        // glDisable(GL_BLEND);
    }

    shader->Use();

    if (texture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(texture->GetGLType(), texture->GetGLTextureID());
        glUniform1i(glGetUniformLocation(shader->program, "WIST_TEXTURE"), 0);
        glUniform1i(glGetUniformLocation(shader->program, "WIST_USE_TEXTURE"), 1);
    } else {
        glUniform1i(glGetUniformLocation(shader->program, "WIST_USE_TEXTURE"), 0);
    }
    GLint loc_ambient = glGetUniformLocation(shader->program, "WIST_AMBIENT_LIGHT");
    glUniform3fv(loc_ambient, 1, glm::value_ptr(Light::ambientLight));

    GLint loc_diffuse = glGetUniformLocation(shader->program, "WIST_MATERIAL_DIFFUSE");
    GLint loc_specular = glGetUniformLocation(shader->program, "WIST_MATERIAL_SPECULAR");
    GLint loc_shininess = glGetUniformLocation(shader->program, "WIST_MATERIAL_SHININESS");
    glUniform3fv(loc_diffuse, 1, glm::value_ptr(diffuseLight));
    glUniform3fv(loc_specular, 1, glm::value_ptr(specularLight));
    glUniform1f(loc_shininess, shininess);

    int textureIdx = !!texture;
    for (auto [name, uniform] : uniforms) {
        auto [type, value] = uniform;
        GLint location = glGetUniformLocation(shader->program, name.c_str());
        if (type == INT) {
            glUniform1i(location, value.intValue);
        } else if (type == FLOAT) {
            glUniform1f(location, value.floatValue);
        } else if (type == IVEC2) {
            glUniform2iv(location, 1, glm::value_ptr(value.ivec2Value));
        } else if (type == VEC2) {
            glUniform2fv(location, 1, glm::value_ptr(value.vec2Value));
        } else if (type == VEC3) {
            glUniform3fv(location, 1, glm::value_ptr(value.vec3Value));
        } else if (type == VEC4) {
            glUniform4fv(location, 1, glm::value_ptr(value.vec4Value));
        } else if (type == MAT3) {
            glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value.mat3Value));
        } else if (type == MAT4) {
            glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value.mat4Value));
        } else if (type == TEXTURE) {
            glActiveTexture(GL_TEXTURE0 + textureIdx);
            glBindTexture(value.textureValue->GetGLType(), value.textureValue->GetGLTextureID());
            glUniform1i(location, textureIdx++);
        }
    }
}



