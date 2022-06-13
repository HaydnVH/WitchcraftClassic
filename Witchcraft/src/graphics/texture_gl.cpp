#ifdef RENDERER_OPENGL
#include "texture.h"
#include <GL/glew.h>
#include <GL/GL.h>
//#include "ext_gl/extensions.h"

void Texture::Clean()
{
	if (tex)
	{
		glDeleteTextures(1, &tex);
		tex = 0;
	}
}

bool Texture::isReady()
{
	return (tex != 0);
}

void Texture::LoadPixels(unsigned int width, unsigned int height, TexEnum compression, TexEnum format, TexEnum type, void* pixels)
{
	// Transform the TexEnum to a GLenum
	GLenum glfmt;
	switch (format)
	{
	case TEXFMT_RED:	glfmt = GL_RED;				break;
	case TEXFMT_RG:		glfmt = GL_RG;				break;
	case TEXFMT_RGB:	glfmt = GL_RGB;				break;
	case TEXFMT_SRGB:	glfmt = GL_RGB;				break;
	case TEXFMT_RGBA:	glfmt = GL_RGBA;			break;
	case TEXFMT_SRGBA:	glfmt = GL_RGBA;			break;
	default:			glfmt = GL_INVALID_ENUM;	break;
	}

	GLenum ifmt;
	switch (compression)
	{
	case TEX_COMPRESS_S3TC_RGB_DXT1:	ifmt = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;			break;
	case TEX_COMPRESS_S3TC_SRGB_DXT1:	ifmt = GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;		break;
	case TEX_COMPRESS_S3TC_RGBA_DXT1:	ifmt = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;		break;
	case TEX_COMPRESS_S3TC_SRGBA_DXT1:	ifmt = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;	break;
	case TEX_COMPRESS_S3TC_RGBA_DXT5:	ifmt = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;		break;
	case TEX_COMPRESS_S3TC_SRGBA_DXT5:	ifmt = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;	break;
	case TEX_COMPRESS_RGTC_RED:			ifmt = GL_COMPRESSED_RED_RGTC1;					break;
	case TEX_COMPRESS_RGTC_RG:			ifmt = GL_COMPRESSED_RG_RGTC2;					break;
	default:
	{
		switch (format)
		{
		case TEXFMT_RED:	ifmt = GL_R8;			break;
		case TEXFMT_RG:		ifmt = GL_RG8;			break;
		case TEXFMT_RGB:	ifmt = GL_RGB8;			break;
		case TEXFMT_SRGB:	ifmt = GL_SRGB8;		break;
		case TEXFMT_RGBA:	ifmt = GL_RGBA8;		break;
		case TEXFMT_SRGBA:	ifmt = GL_SRGB8_ALPHA8;	break;
		default:			ifmt = glfmt;			break;
		}
	}
	}

	// Create the texture handle.
	if (!tex)
	{
		glGenTextures(1, &tex);
	}
	glBindTexture(GL_TEXTURE_2D, tex);

	// Ensure that we don't try to access mipmaps that don't exist.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	// Load the pixels.
	glTexImage2D(GL_TEXTURE_2D, 0, ifmt, width, height, 0, glfmt, GL_BYTE + (int)type, pixels);
}

void Texture::LoadCubemapPixels(unsigned int width, unsigned int height, TexEnum compression, TexEnum format, TexEnum type, void* pixels[])
{
	// Transform the TexEnum to a GLenum
	GLenum glfmt;
	switch (format)
	{
	case TEXFMT_RED:	glfmt = GL_RED;				break;
	case TEXFMT_RG:		glfmt = GL_RG;				break;
	case TEXFMT_RGB:	glfmt = GL_RGB;				break;
	case TEXFMT_SRGB:	glfmt = GL_RGB;				break;
	case TEXFMT_RGBA:	glfmt = GL_RGBA;			break;
	case TEXFMT_SRGBA:	glfmt = GL_RGBA;			break;
	default:			glfmt = GL_INVALID_ENUM;	break;
	}

	GLenum ifmt;
	switch (compression)
	{
	case TEX_COMPRESS_S3TC_RGB_DXT1:	ifmt = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;			break;
	case TEX_COMPRESS_S3TC_SRGB_DXT1:	ifmt = GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;		break;
	case TEX_COMPRESS_S3TC_RGBA_DXT1:	ifmt = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;		break;
	case TEX_COMPRESS_S3TC_SRGBA_DXT1:	ifmt = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;	break;
	case TEX_COMPRESS_S3TC_RGBA_DXT5:	ifmt = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;		break;
	case TEX_COMPRESS_S3TC_SRGBA_DXT5:	ifmt = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;	break;
	case TEX_COMPRESS_RGTC_RED:			ifmt = GL_COMPRESSED_RED_RGTC1;					break;
	case TEX_COMPRESS_RGTC_RG:			ifmt = GL_COMPRESSED_RG_RGTC2;					break;
	default:
	{
		switch (format)
		{
		case TEXFMT_RED:	ifmt = GL_R8;			break;
		case TEXFMT_RG:		ifmt = GL_RG8;			break;
		case TEXFMT_RGB:	ifmt = GL_RGB8;			break;
		case TEXFMT_SRGB:	ifmt = GL_SRGB8;		break;
		case TEXFMT_RGBA:	ifmt = GL_RGBA8;		break;
		case TEXFMT_SRGBA:	ifmt = GL_SRGB8_ALPHA8;	break;
		default:			ifmt = glfmt;			break;
		}
	}
	}

	// Create the texture handle.
	if (!tex)
	{
		glGenTextures(1, &tex);
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

	// Ensure that we don't try to access mipmaps that don't exist.
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);

	// Load the pixels.
	for (int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, ifmt, width, height, 0, glfmt, GL_BYTE + (int)type, pixels[i]);
	}
}

void Texture::LoadSubPixels(int xoffset, int yoffset, unsigned int width, unsigned int height, TexEnum format, TexEnum type, void* pixels)
{
	if (tex == 0) return;
	glBindTexture(GL_TEXTURE_2D, tex);

	// Transform the TexEnum to a GLenum
	GLenum glfmt;
	switch (format)
	{
	case TEXFMT_RED:	glfmt = GL_RED;				break;
	case TEXFMT_RG:		glfmt = GL_RG;				break;
	case TEXFMT_RGB:	glfmt = GL_RGB;				break;
	case TEXFMT_SRGB:	glfmt = GL_SRGB;			break;
	case TEXFMT_RGBA:	glfmt = GL_RGBA;			break;
	case TEXFMT_SRGBA:	glfmt = GL_SRGB_ALPHA;		break;
	default:			glfmt = GL_INVALID_ENUM;	break;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, glfmt, GL_BYTE + (int)type, pixels);
}

void Texture::GenerateMipmaps()
{
	if (!tex) return;
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1000);
	glGenerateMipmap(GL_TEXTURE_2D);
}

bool Texture::Use2D(unsigned int index)
{
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, tex);
	return (tex != 0);
}

bool Texture::UseCube(unsigned int index)
{
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	return (tex != 0);
}

void Texture::setFiltering(TexEnum min, TexEnum mag, float aniso)
{
	if (tex == 0) return;
	glBindTexture(GL_TEXTURE_2D, tex);

	if (min == TEXFILTER_LINEAR)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

	if (mag == TEXFILTER_LINEAR)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (aniso > 0.0f)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
}

void Texture::setWrapping(TexEnum s, TexEnum t)
{
	if (tex == 0) return;
	glBindTexture(GL_TEXTURE_2D, tex);

	if (s)		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	else		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	if (t)		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	else		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}


#endif // RENDERER_OPENGL