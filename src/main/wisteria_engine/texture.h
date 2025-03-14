#pragma once
#include <iostream>
#include <GL/glew.h>
#include "stb/stb_image.h"

/// symbolic constant for internal constructors to disambiguate argument lists
#define INTERNAL true

namespace engine
{
    typedef unsigned char byte;
    class Texture
    {
    public:
        virtual ~Texture();

        int GetWidth() { return width; }
        int GetHeight() { return height; }

        virtual GLenum GetGLType() = 0;
        GLuint GetGLTextureID() { return textureID; }
        byte *GetRawTextureData() { return data; }

        struct Format {
            GLint internal;
            GLenum input;
            GLenum type;
        };
        inline static const Format RGB = { GL_RGB, GL_RGB, GL_UNSIGNED_BYTE };
        inline static const Format RGBA = { GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE };
        inline static const Format RGBA32F = { GL_RGBA32F, GL_RGBA, GL_FLOAT };
        inline static const Format DEPTHF = { GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT };
        inline static const Format DEPTH32F = { GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT };

    protected:
        Texture(int width, int height, Format format = RGB);

        void TexImage2D(GLuint glType, void *data);

        virtual void Bind() = 0;
        virtual void UnBind() = 0;

        GLuint textureID;
        Format format;
        int width;
        int height;
        byte *data = nullptr;
    };

    class Texture2D : public Texture
    {
    public:
        Texture2D(int width, int height, Format format = RGB);

        virtual GLenum GetGLType() override { return GL_TEXTURE_2D; }

        static Texture2D *Load(const char *fileName);

    protected:
        Texture2D(bool _internal, int width, int height, 
                  Format format = RGB, byte *data = nullptr);

        void Bind() override;
        void UnBind() override;
    };

    class DepthTexture2D : public Texture
    {
    public:
        DepthTexture2D(int width, int height, Format format = DEPTHF);

        virtual GLenum GetGLType() override { return GL_TEXTURE_2D; }
    
    protected:
        void Bind() override;
        void UnBind() override;
    };

    class Cubemap : public Texture
    {
    public:
        Cubemap(int width, int height, Format format = RGB);

        virtual GLenum GetGLType() override { return GL_TEXTURE_CUBE_MAP; }

        static Cubemap *Load(const char *pos_x, const char *neg_x, const char *pos_y, 
                             const char *neg_y, const char *pos_z, const char *neg_z);

    protected:
        Cubemap(bool _internal, int width, int height, Format format = RGB,
                byte *data_pos_x = nullptr, byte *data_neg_x = nullptr,
                byte *data_pos_y = nullptr, byte *data_neg_y = nullptr,
                byte *data_pos_z = nullptr, byte *data_neg_z = nullptr);

        void Bind() override;
        void UnBind() override;

    private:

    };

    class DepthCubemap : public Texture
    {
    public:
        DepthCubemap(int width, int height, Format format = DEPTHF);

        virtual GLenum GetGLType() override { return GL_TEXTURE_CUBE_MAP; }

    protected:
        void Bind() override;
        void UnBind() override;

    };
}