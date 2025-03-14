#pragma once
#include "./controlledscene3d.h"
#include "./texture.h"

namespace engine 
{
    class FrameBuffer 
    {
    friend class ControlledScene3D;
    public:
        enum Direction { NONE, TEX2D, CUBE, CUBE_LAYERED };
        struct Shape;

        FrameBuffer(int width, int height);
        FrameBuffer(Shape shape);
        virtual ~FrameBuffer();

        Texture *GetColorTexture(unsigned int index);
        Texture *GetDepthTexture();

        int GetWidth() { return shape.width; }
        int GetHeight() { return shape.height; }
        Shape GetShape() { return shape; }
        bool NeedsCubeRendering() { return anyCube; }

        void SetColorTexture(unsigned char numTexture, Texture::Format format = Texture::RGB,
                             Direction direction = TEX2D);
        void SetColorTexture(unsigned char numTexture, Direction direction);
        void SetDepthTexture(bool hasDepthTexture, Texture::Format format = Texture::DEPTHF,
                             Direction direction = TEX2D);
        void SetDepthTexture(bool hasDepthTexture, Direction direction);
        /// @brief Change the render target of a cubemap texture from this framebuffer to
        ///        a specific face
        /// @param numTexture The index of the color texture. The texture must be a cubemap texture
        /// @param face The index of the face (0-5) in order (+X, -X, +Y, -Y, +Z, -Z)
        void AttachCubemapFace(unsigned int numTexture, unsigned int face);
        void AttachDepthCubemapFace(unsigned int face);
        void ClearColor(glm::vec4 clearColor) { this->clearColor = clearColor; }
        void Clear(bool color = true, bool depth = true, std::set<unsigned char> attachments = {});

        unsigned int fbo = 0;  // TODO: when done debugging, move this back to protected

    protected:
        void Complete();
        void Bind();
        void UnBind();
        void CheckLayered(Direction direction);

        bool anyCube = false;  // whether any of the textures is an unlayered cubemap
        enum Layered { LAYERED_UNKNOWN = -1, UNLAYERED = 0, LAYERED = 1 };
        Layered layered = LAYERED_UNKNOWN;  // whether any of the textures is layered or unlayered
        // a framebuffer cannot contain both layered and unlayered textures

        std::vector<Texture *> colorTextures;
        Texture *depthTexture = nullptr;
        glm::vec4 clearColor = glm::vec4(0, 0, 0, 1);

    public:
        struct TextureDesc {
            Texture::Format format;
            Direction direction;
        };

        struct Shape {
            int width, height;
            std::set<unsigned char> colorAttachments;
            std::vector<TextureDesc> colorDescriptors;
            TextureDesc depthDescriptor = { Texture::DEPTHF, NONE };
        } shape;
    };
};