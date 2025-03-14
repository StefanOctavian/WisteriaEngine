#include "./framebuffer.h"

using namespace engine;

FrameBuffer::FrameBuffer(int width, int height): shape({width, height})
{
    int maxColorAttachments;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
    colorTextures = std::vector<Texture *>(maxColorAttachments, nullptr);
    shape.colorDescriptors = std::vector<TextureDesc>(maxColorAttachments, { 0, NONE });

    glGenFramebuffers(1, &fbo);
}

FrameBuffer::FrameBuffer(Shape shape): FrameBuffer(shape.width, shape.height)
{
    this->shape.colorAttachments = shape.colorAttachments;
    this->shape.colorDescriptors = shape.colorDescriptors;
    this->shape.depthDescriptor = shape.depthDescriptor;
    if (shape.depthDescriptor.direction != NONE) {
        this->layered = (Layered)(shape.depthDescriptor.direction == CUBE_LAYERED);
    } else if (!shape.colorAttachments.empty()) {
        this->layered = (Layered)(shape.colorDescriptors[0].direction == CUBE_LAYERED);
    } else {
        this->layered = LAYERED_UNKNOWN;
    }
    for (auto att : shape.colorAttachments) {
        if (shape.colorDescriptors[att].direction == NONE) continue;
        SetColorTexture(att, shape.colorDescriptors[att].format, shape.colorDescriptors[att].direction);
        anyCube |= (shape.colorDescriptors[att].direction == CUBE);
    }
    if (shape.depthDescriptor.direction != NONE) {
        SetDepthTexture(true, shape.depthDescriptor.format, shape.depthDescriptor.direction);
        anyCube |= (shape.depthDescriptor.direction == CUBE);
    }
}

FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &fbo);
    for (auto texture : colorTextures) {
        if (texture) delete texture;
    }
    if (depthTexture) delete depthTexture;
}

Texture *FrameBuffer::GetColorTexture(unsigned int index)
{
    return colorTextures[index];
}

Texture *FrameBuffer::GetDepthTexture()
{
    return depthTexture;
}

void FrameBuffer::CheckLayered(Direction direction)
{
    bool addLayered = (direction == CUBE_LAYERED);
    if ((layered == LAYERED && !addLayered) || (layered == UNLAYERED && addLayered)) {
        std::cout << "A FrameBuffer cannot contain both layered and unlayered textures";
        std::abort();
    } else if (layered == LAYERED_UNKNOWN) {
        layered = (Layered)addLayered;
    }
}

void FrameBuffer::SetColorTexture(unsigned char numTexture, Texture::Format format, Direction direction)
{
    CheckLayered(direction);
    anyCube |= (direction == CUBE);
    colorTextures[numTexture] = direction == TEX2D ?
        (Texture *)new Texture2D(shape.width, shape.height, format) :
        (Texture *)new Cubemap(shape.width, shape.height, format);

    GLuint textureID = colorTextures[numTexture]->GetGLTextureID();
    Bind();
        if (layered != LAYERED)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + numTexture,
                                   direction == CUBE ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : GL_TEXTURE_2D,
                                   textureID, 0);
        else glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + numTexture, textureID, 0);

        shape.colorAttachments.insert(numTexture);
        shape.colorDescriptors[numTexture] = { format, direction };
        Complete();
    UnBind();
}
void FrameBuffer::SetColorTexture(unsigned char numTexture, Direction direction) {
    SetColorTexture(numTexture, Texture::RGB, direction);
}

void FrameBuffer::AttachCubemapFace(unsigned int numTexture, unsigned int face)
{
    if (shape.colorDescriptors[numTexture].direction != CUBE) {
        std::cout << "Cannot attach face. Texture is not an unlayered cubemap texture";
        std::abort();
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + numTexture, 
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 
                           colorTextures[numTexture]->GetGLTextureID(), 0);
}

void FrameBuffer::AttachDepthCubemapFace(unsigned int face)
{
    if (shape.depthDescriptor.direction != CUBE) {
        std::cout << "Cannot attach face. Texture is not an unlayered cubemap texture";
        std::abort();
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 
                           depthTexture->GetGLTextureID(), 0);
}

void FrameBuffer::Clear(bool color, bool depth, std::set<unsigned char> attachments)
{
    if (!color && !depth) return;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    for (int face = 0; face < 6; ++face) {
        if (color) {
            for (auto att : shape.colorAttachments) {
                if (shape.colorDescriptors[att].direction != CUBE) continue;
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att, 
                                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                                       colorTextures[att]->GetGLTextureID(), 0);
            }
        }
        if (depth && shape.depthDescriptor.direction == CUBE) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 
                                   depthTexture->GetGLTextureID(), 0);
        }
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        for (auto att : attachments) {
            glClearBufferfv(GL_COLOR, att, glm::value_ptr(clearColor));
        }
        glClear((color ? GL_COLOR_BUFFER_BIT : 0x0) | (depth ? GL_DEPTH_BUFFER_BIT : 0x0));
        glViewport(0, 0, shape.width, shape.height);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::Complete()
{
    std::vector<GLenum> attachments;
    attachments.reserve(shape.colorAttachments.size());
    for (auto att : shape.colorAttachments) {
        attachments.push_back(GL_COLOR_ATTACHMENT0 + att);
    }

    glDrawBuffers((GLsizei)attachments.size(), attachments.data());
    checkFBStatus();
}

void FrameBuffer::SetDepthTexture(bool hasDepthTexture, Texture::Format format, Direction direction)
{
    if (!this->depthTexture == !hasDepthTexture)
        return;
    CheckLayered(direction);

    if (!hasDepthTexture) {
        delete depthTexture;
        depthTexture = nullptr;
        return;
    }

    anyCube |= (direction == CUBE);
    depthTexture = direction == TEX2D ?
        (Texture *)new DepthTexture2D(shape.width, shape.height, format) :
        (Texture *)new DepthCubemap(shape.width, shape.height, format);

    GLuint textureID = depthTexture->GetGLTextureID();
    Bind();
        if (layered != LAYERED)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                                   direction == CUBE ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : GL_TEXTURE_2D, 
                                   depthTexture->GetGLTextureID(), 0);
        else glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureID, 0);
            
        shape.depthDescriptor = { format, direction };
        checkFBStatus();
    UnBind();
}
void FrameBuffer::SetDepthTexture(bool hasDepthTexture, Direction direction) {
    SetDepthTexture(hasDepthTexture, Texture::DEPTHF, direction);
}

void FrameBuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, shape.width, shape.height);
}

void FrameBuffer::UnBind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, shape.width, shape.height);
}