#pragma once

#include <GLES3/gl32.h>
#include <GLES/gl.h>

#include <iostream>

class Texture2D
{
public:
    Texture2D()
    {
        glGenTextures(1, &mTexHandle);
    }

    virtual ~Texture2D()
    {
        std::cout << __func__ << std::endl;
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    bool bind()
    {
        glBindTexture(GL_TEXTURE_2D, mTexHandle);
        return true;
    }

    bool unbind()
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        return true;
    }

    bool loadImage( GLint level, GLint internalformat,
        GLsizei width, GLsizei height, GLint border,
        GLenum format, GLenum type,
        const void * data)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexImage2D(GL_TEXTURE_2D, level, internalformat, width, height, border, format, type, data);
        return true;
    }

    GLuint handle() const
    {
        return mTexHandle;
    }

private:
    GLuint mTexHandle;
};
