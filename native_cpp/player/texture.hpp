#pragma once

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>

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
        unbind();
        glDeleteTextures(1, &mTexHandle);
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE /*GL_REPEAT*/ );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE /*GL_REPEAT*/ );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

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
