#pragma once

#include "utils/gl_utils.h"
#include <string>

class Texture2D
{
public:
   Texture2D();
   ~Texture2D();

   void Bind() const;
   void BindToTextureUnit(GLenum TextureUnit) const;
   void UnBind() const;

   void UploadNewData(const unsigned char *img);
   void UploadNewData(const unsigned int *img);

   void Init(GLuint gpuTextureID, unsigned int width, unsigned int height, unsigned int channels);
   void Create(const unsigned char *img, int width, int height, int chn);
   void CreateU16(const unsigned int *img, int width, int height, int chn);

   void InitCubeTexture(unsigned int width, unsigned int height,
                        const unsigned char *data_pos_x, const unsigned char *data_pos_y,
                        const unsigned char *data_pos_z, const unsigned char *data_neg_x,
                        const unsigned char *data_neg_y, const unsigned char *data_neg_z);

   void LoadCubeTexture(const char *pos_x, const char *pos_y, const char *pos_z,
                        const char *neg_x, const char *neg_y, const char *neg_z);
   void CreateFrameBufferTexture(unsigned int width, unsigned int height, unsigned int targetID);
   void CreateDepthBufferTexture(unsigned int width, unsigned int height);
   void CreateFrameBufferCubeMapTexture(unsigned int width, unsigned int height);

   bool Load2D(const char *fileName, GLenum wrappingMode = GL_REPEAT);
   void SaveToFile(const char *fileName);
   void SaveCubemapFacesDirect(std::string filename);
   void CacheInMemory(bool state);

   unsigned int GetWidth() const;
   unsigned int GetHeight() const;
   void GetSize(unsigned int &width, unsigned int &height) const;
   unsigned char *GetImageData() const;

   unsigned int GetNrChannels() const;

   void SetWrappingMode(GLenum mode);
   void SetFiltering(GLenum minFilter, GLenum magFilter = GL_LINEAR);

   GLuint GetTextureID() const;

private:
   void SetTextureParameters();
   void Init2DTexture(unsigned int width, unsigned int height, unsigned int channels);

private:
   bool cacheInMemory;
   unsigned int bitsPerPixel;
   unsigned int width;
   unsigned int height;
   unsigned int channels;

   GLuint targetType;
   GLuint textureID;
   GLenum wrappingMode;
   GLenum textureMinFilter;
   GLenum textureMagFilter;

   unsigned char *imageData;
};
