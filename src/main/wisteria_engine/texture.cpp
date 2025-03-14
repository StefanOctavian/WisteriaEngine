#include "texture.h"

using namespace engine;

Texture::Texture(int width, int height, Format format): width(width), height(height), format(format)
{
    glGenTextures(1, &textureID);
}
Texture::~Texture() { 
    glDeleteTextures(1, &textureID);
    delete[] data;
}

void Texture::TexImage2D(GLuint glType, void *data)
{
    glTexImage2D(glType, 0, format.internal, width, height, 0, format.input, format.type, data);
}

// ----------------------------------------------------------------------------


Texture2D::Texture2D(bool _internal, int width, int height, 
                     Format format, byte *data): Texture(width, height, format)
{
    Bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (GLEW_EXT_texture_filter_anisotropic) {
            float maxAnisotropy;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
        }
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        TexImage2D(GL_TEXTURE_2D, data);
    UnBind();

    this->data = (data == nullptr) ? new byte[width * height * 3] {0} : data;
}
Texture2D::Texture2D(int width, int height, Format format): Texture2D(INTERNAL, width, height, format) {}

void Texture2D::Bind() { glBindTexture(GL_TEXTURE_2D, textureID); }
void Texture2D::UnBind() { glBindTexture(GL_TEXTURE_2D, 0); }

Texture2D *Texture2D::Load(const char *fileName)
{
    int width, height;
    byte *data = stbi_load(fileName, &width, &height, nullptr, 3);

    if (data == nullptr) {
        std::cout << "Error loading texture: " << fileName << "\n" << std::endl;
        std::abort();
    }
    return new Texture2D(INTERNAL, width, height, RGB, data);
}


// ----------------------------------------------------------------------------


DepthTexture2D::DepthTexture2D(int width, int height, Format format): Texture(width, height, format)
{
    Bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        TexImage2D(GL_TEXTURE_2D, nullptr);
    UnBind();
}

void DepthTexture2D::Bind() { glBindTexture(GL_TEXTURE_2D, textureID); }
void DepthTexture2D::UnBind() { glBindTexture(GL_TEXTURE_2D, 0); }


// ----------------------------------------------------------------------------

Cubemap::Cubemap(bool _internal, int width, int height, Format format,
                 byte *data_pos_x, byte *data_neg_x, byte *data_pos_y,
                 byte *data_neg_y, byte *data_pos_z, byte *data_neg_z): Texture(width, height, format)
{
    Bind();
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, data_pos_x);
        TexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, data_neg_x);
        TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, data_pos_y);
        TexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, data_neg_y);
        TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, data_pos_z);
        TexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, data_neg_z);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (GLEW_EXT_texture_filter_anisotropic) {
            float maxAnisotropy;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
        }
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        // glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    UnBind();

    data = new byte[width * height * 3 * 6] {0};

    if (data_pos_x) memcpy(data, data_pos_x, width * height * 3);
    if (data_neg_x) memcpy(data + width * height * 3, data_neg_x, width * height * 3);
    if (data_pos_y) memcpy(data + width * height * 3 * 2, data_pos_y, width * height * 3);
    if (data_neg_y) memcpy(data + width * height * 3 * 3, data_neg_y, width * height * 3);
    if (data_pos_z) memcpy(data + width * height * 3 * 4, data_pos_z, width * height * 3);
    if (data_neg_z) memcpy(data + width * height * 3 * 5, data_neg_z, width * height * 3);
}
Cubemap::Cubemap(int width, int height, Format format): Cubemap(INTERNAL, width, height, format) {}

void Cubemap::Bind() { glBindTexture(GL_TEXTURE_CUBE_MAP, textureID); }
void Cubemap::UnBind() { glBindTexture(GL_TEXTURE_CUBE_MAP, 0); }

Cubemap *Cubemap::Load(const char *pos_x, const char *neg_x, const char *pos_y, 
                       const char *neg_y, const char *pos_z, const char *neg_z)
{
    int width, height;
    byte *data_pos_x = stbi_load(pos_x, &width, &height, nullptr, 3);
    byte *data_neg_x = stbi_load(neg_x, &width, &height, nullptr, 3);
    byte *data_pos_y = stbi_load(pos_y, &width, &height, nullptr, 3);
    byte *data_neg_y = stbi_load(neg_y, &width, &height, nullptr, 3);
    byte *data_pos_z = stbi_load(pos_z, &width, &height, nullptr, 3);
    byte *data_neg_z = stbi_load(neg_z, &width, &height, nullptr, 3);

    if (data_pos_x == nullptr || data_neg_x == nullptr || data_pos_y == nullptr ||
        data_neg_y == nullptr || data_pos_z == nullptr || data_neg_z == nullptr) {
        std::cout << "Error loading cubemap texture\n" << std::endl;
        std::abort();
    }

    auto *cubemap = new Cubemap(INTERNAL, width, height, RGB, 
                                data_pos_x, data_neg_x, data_pos_y, 
                                data_neg_y, data_pos_z, data_neg_z);

    stbi_image_free(data_pos_x);
    stbi_image_free(data_neg_x);
    stbi_image_free(data_pos_y);
    stbi_image_free(data_neg_y);
    stbi_image_free(data_pos_z);
    stbi_image_free(data_neg_z);

    return cubemap;
}

// ----------------------------------------------------------------------------


DepthCubemap::DepthCubemap(int width, int height, Format format): Texture(width, height, format)
{
    Bind();
        TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, nullptr);
        TexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, nullptr);
        TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, nullptr);
        TexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, nullptr);
        TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, nullptr);
        TexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, nullptr);
    UnBind();
}

void DepthCubemap::Bind() { glBindTexture(GL_TEXTURE_CUBE_MAP, textureID); }
void DepthCubemap::UnBind() { glBindTexture(GL_TEXTURE_CUBE_MAP, 0); }