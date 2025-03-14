#include "core/gpu/texture2D.h"

#include <thread>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#include "utils/memory_utils.h"


void write_image_thread(const char* fileName, unsigned int width, unsigned int height, unsigned int channels, const unsigned char *data)
{
    stbi_write_png(fileName, width, height, channels, data, width * channels);
    delete data;
}


const GLint pixelFormat[5] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
const GLint internalFormat[][5] = {
    { 0, GL_R8, GL_RG8, GL_RGB8, GL_RGBA8 },
    { 0, GL_R16, GL_RG16, GL_RGB16, GL_RGBA16 },
    { 0, GL_R16F, GL_RG16F, GL_RGB16F, GL_RGBA16F },
    { 0, GL_R32F, GL_RG32F, GL_RGB32F, GL_RGBA32F }
};


Texture2D::Texture2D()
{
    width = 0;
    height = 0;
    channels = 0;
    textureID = 0;
    bitsPerPixel = 8;
    cacheInMemory = false;
    targetType = GL_TEXTURE_2D;
    wrappingMode = GL_REPEAT;
    textureMinFilter = GL_LINEAR;
    textureMagFilter = GL_LINEAR;
}


Texture2D::~Texture2D() {
}


GLuint Texture2D::GetTextureID() const
{
    return textureID;
}


void Texture2D::Init(GLuint gpuTextureID, unsigned int width, unsigned int height, unsigned int channels)
{
    this->textureID = gpuTextureID;
    this->width = width;
    this->height = height;
    this->channels = channels;
}


bool Texture2D::Load2D(const char *fileName, GLenum wrapping_mode)
{
    int width, height, chn;
    imageData = stbi_load(fileName, &width, &height, &chn, 0);

    if (imageData == NULL) {
#ifdef DEBUG_INFO
        std::cout << "ERROR loading texture: " << fileName << "\n\n";
#endif
        return false;
    }

#ifdef DEBUG_INFO
    std::cout << "Loaded " << fileName << "\n";
    std::cout << width << " * " << height << " channels: " << chn << "\n\n";
#endif

    textureMinFilter = GL_LINEAR_MIPMAP_LINEAR;
    wrappingMode = wrapping_mode;

    Init2DTexture(width, height, chn);
    glTexImage2D(targetType, 0, internalFormat[0][chn], width, height, 0, pixelFormat[chn], GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(targetType);
    glBindTexture(targetType, 0);
    CheckOpenGLError();

    if (cacheInMemory == false)
    {
        stbi_image_free(imageData);
    }

    return true;
}


void Texture2D::SaveToFile(const char *fileName)
{
    if (imageData == nullptr)
    {
        imageData = new unsigned char[width * height * channels];
    }
    glBindTexture(targetType, textureID);
    glGetTexImage(targetType, 0, pixelFormat[channels], GL_UNSIGNED_BYTE, (void *)imageData);

    stbi_write_png(fileName, width, height, channels, imageData, width * channels);
}

// Function to extract and save cubemap faces
void Texture2D::SaveCubemapFacesDirect(std::string filename) {
    // List of cubemap face targets
    GLenum cubemapFaces[6] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    const char* faceNames[6] = { "posx", "negx", "posy", "negy", "posz", "negz" };

    // Buffer to hold the pixel data for a single face
    unsigned char *pixels = new unsigned char[width * height * 3]; // RGB

    // Bind the cubemap texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    // Retrieve and save each face
    for (int i = 0; i < 6; ++i) {
        // Get the pixel data for the current face
        glGetTexImage(cubemapFaces[i], 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

        // Flip the image vertically
        for (unsigned int y = 0; y < width / 2; ++y) {
            for (unsigned int x = 0; x < height * 3; ++x) {
                std::swap(pixels[y * width * 3 + x], pixels[(width - 1 - y) * height * 3 + x]);
            }
        }

        // Save the face to a file
        std::string fileName = filename + faceNames[i] + ".png";
        write_image_thread(fileName.c_str(), width, height, 3, pixels);
    }
}



void Texture2D::CacheInMemory(bool state)
{
    cacheInMemory = state;
}


void Texture2D::UploadNewData(const unsigned char *img)
{
    Bind();
    glTexSubImage2D(targetType, 0, 0, 0, width, height, pixelFormat[channels], GL_UNSIGNED_BYTE, img);
    glGenerateMipmap(targetType);
    UnBind();
}


void Texture2D::UploadNewData(const unsigned int *img)
{
    Bind();
    glTexSubImage2D(targetType, 0, 0, 0, width, height, pixelFormat[channels], GL_UNSIGNED_INT, img);
    glGenerateMipmap(targetType);
    UnBind();
}


void Texture2D::Create(const unsigned char *img, int width, int height, int chn)
{
    Init2DTexture(width, height, chn);
    glTexImage2D(targetType, 0, internalFormat[0][chn], width, height, 0, pixelFormat[chn], GL_UNSIGNED_BYTE, (void *)img);
    UnBind();
}


void Texture2D::CreateU16(const unsigned int *img, int width, int height, int chn)
{
    Init2DTexture(width, height, chn);
    glTexImage2D(targetType, 0, internalFormat[1][chn], width, height, 0, pixelFormat[chn], GL_UNSIGNED_INT, (void *)img);
    UnBind();
}

void Texture2D::InitCubeTexture(unsigned int width, unsigned int height,
                                const unsigned char *data_pos_x, const unsigned char *data_pos_y, 
                                const unsigned char *data_pos_z, const unsigned char *data_neg_x,
                                const unsigned char *data_neg_y, const unsigned char *data_neg_z)
{
    this->width = width;
    this->height = height;
    targetType = GL_TEXTURE_CUBE_MAP;
    textureMinFilter = GL_LINEAR_MIPMAP_LINEAR;
    wrappingMode = GL_CLAMP_TO_EDGE;

    Init2DTexture(width, height, 3);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_z);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_z);

    glGenerateMipmap(targetType);

    UnBind();
}


void Texture2D::LoadCubeTexture(const char *pos_x, const char *pos_y, const char *pos_z, 
                                const char *neg_x, const char *neg_y, const char *neg_z)
{
    int width, height, chn;
    unsigned char *data_pos_x = stbi_load(pos_x, &width, &height, &chn, 0);
    unsigned char *data_pos_y = stbi_load(pos_y, &width, &height, &chn, 0);
    unsigned char *data_pos_z = stbi_load(pos_z, &width, &height, &chn, 0);
    unsigned char *data_neg_x = stbi_load(neg_x, &width, &height, &chn, 0);
    unsigned char *data_neg_y = stbi_load(neg_y, &width, &height, &chn, 0);
    unsigned char *data_neg_z = stbi_load(neg_z, &width, &height, &chn, 0);

    InitCubeTexture(width, height, data_pos_x, data_pos_y, data_pos_z, 
                    data_neg_x, data_neg_y, data_neg_z);

    if (cacheInMemory == false)
    {
        SAFE_FREE(data_pos_x);
        SAFE_FREE(data_pos_y);
        SAFE_FREE(data_pos_z);
        SAFE_FREE(data_neg_x);
        SAFE_FREE(data_neg_y);
        SAFE_FREE(data_neg_z);
    }
}


void Texture2D::CreateFrameBufferTexture(unsigned int width, unsigned int height, unsigned int targetID)
{
    Init2DTexture(width, height, 4);
    glTexImage2D(targetType, 0, internalFormat[0][3], width, height, 0, pixelFormat[3], GL_UNSIGNED_BYTE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + targetID, GL_TEXTURE_2D, textureID, 0);
    UnBind();
}

void Texture2D::CreateFrameBufferCubeMapTexture(unsigned int width, unsigned int height)
{
    InitCubeTexture(width, height, 0, 0, 0, 0, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, textureID, 0);
    UnBind();
}

void Texture2D::CreateDepthBufferTexture(unsigned int width, unsigned int height)
{
    Init2DTexture(width, height, 1);
    glTexImage2D(targetType, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);
    UnBind();
}


void Texture2D::Bind() const
{
    glBindTexture(targetType, textureID);
}


void Texture2D::BindToTextureUnit(GLenum TextureUnit) const
{
    if (!textureID) return;
    glActiveTexture(TextureUnit);
    glBindTexture(targetType, textureID);
}


void Texture2D::UnBind() const
{
    glBindTexture(targetType, 0);
    CheckOpenGLError();
}


void Texture2D::SetTextureParameters()
{
    glTexParameteri(targetType, GL_TEXTURE_MIN_FILTER, textureMinFilter);
    glTexParameteri(targetType, GL_TEXTURE_MAG_FILTER, textureMagFilter);
    glTexParameteri(targetType, GL_TEXTURE_WRAP_R, wrappingMode);
    glTexParameteri(targetType, GL_TEXTURE_WRAP_S, wrappingMode);
    glTexParameteri(targetType, GL_TEXTURE_WRAP_T, wrappingMode);
    if (GLEW_EXT_texture_filter_anisotropic) {
        float maxAnisotropy;

        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
    }
}


unsigned int Texture2D::GetWidth() const
{
    return width;
}


unsigned int Texture2D::GetHeight() const
{
    return height;
}


void Texture2D::GetSize(unsigned int &width, unsigned int &height) const
{
    width = this->width;
    height = this->height;
}


unsigned char *Texture2D::GetImageData() const
{
    return imageData;
}


unsigned int Texture2D::GetNrChannels() const
{
    return channels;
}


void Texture2D::SetWrappingMode(GLenum mode)
{
    if (wrappingMode == mode)
        return;

    wrappingMode = mode;

    if (textureID)
    {
        glBindTexture(targetType, textureID);
        glTexParameteri(targetType, GL_TEXTURE_WRAP_S, mode);
        glTexParameteri(targetType, GL_TEXTURE_WRAP_T, mode);
        glTexParameteri(targetType, GL_TEXTURE_WRAP_R, mode);
    }
    CheckOpenGLError();
}


void Texture2D::SetFiltering(GLenum minFilter, GLenum magFilter)
{
    if (textureID)
    {
        glBindTexture(targetType, textureID);

        if (textureMinFilter != minFilter) {
            glTexParameteri(targetType, GL_TEXTURE_MIN_FILTER, minFilter);
            textureMinFilter = minFilter;
        }

        if (textureMagFilter != magFilter) {
            glTexParameteri(targetType, GL_TEXTURE_MAG_FILTER, magFilter);
            textureMagFilter = magFilter;
        }

        CheckOpenGLError();
    }
}


void Texture2D::Init2DTexture(unsigned int width, unsigned int height, unsigned int channels)
{
    this->width = width;
    this->height = height;
    this->channels = channels;

    if (textureID)
        glDeleteTextures(1, &textureID);

    glGenTextures(1, &textureID);
    glBindTexture(targetType, textureID);

    SetTextureParameters();
    if (targetType != GL_TEXTURE_CUBE_MAP)
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
    else
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    CheckOpenGLError();
}
