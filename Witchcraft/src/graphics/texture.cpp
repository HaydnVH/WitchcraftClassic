#include "texture.h"

#include "filesystem/file_manager.h"
#include "sys/printlog.h"
#include "math/vmath.h"

#include <stb_image.h>

unsigned int Texture::LoadImage(const char* filename, bool srgb, bool compress, bool rgba1bit)
{
	// Open the file.
	InFile file = filemanager::LoadSingleFile(filename, std::ios::binary);
	std::string contents = file.contents();

	// Load the image.
	int w, h, channels;
	unsigned char* image = stbi_load_from_memory((unsigned char*)contents.c_str(), (int)contents.size(), &w, &h, &channels, 0);
	if (!image)
	{
		plog::error("In Texture::LoadImage():\n");
		plog::errmore("Couldn't load '%s':\n%s\n", filename, stbi_failure_reason());
		LoadDebug();
		return 0;
	}

	// Figure out the pixel format //
	TexEnum format;
	TexEnum internalFormat;
	switch (channels)
	{
	case 4:
		if (srgb)
		{
			format = TEXFMT_SRGBA;
			if (compress)
			{
				if (rgba1bit)
					internalFormat = TEX_COMPRESS_S3TC_SRGBA_DXT1;
				else
					internalFormat = TEX_COMPRESS_S3TC_SRGBA_DXT5;
			}
			else
				internalFormat = TEX_COMPRESS_NONE;
		}
		else
		{
			format = TEXFMT_RGBA;
			if (compress)
			{
				if (rgba1bit)
					internalFormat = TEX_COMPRESS_S3TC_RGBA_DXT1;
				else
					internalFormat = TEX_COMPRESS_S3TC_RGBA_DXT5;
			}
			else
				internalFormat = TEX_COMPRESS_NONE;
		}
		break;
	case 3:
		if (srgb)
		{
			format = TEXFMT_SRGB;
			if (compress)
				internalFormat = TEX_COMPRESS_S3TC_SRGB_DXT1;
			else
				internalFormat = TEX_COMPRESS_NONE;
		}
		else
		{
			format = TEXFMT_RGB;
			if (compress)
				internalFormat = TEX_COMPRESS_S3TC_RGB_DXT1;
			else
				internalFormat = TEX_COMPRESS_NONE;
		}
		break;
	case 2:
		format = TEXFMT_RG;
		if (compress)
			internalFormat = TEX_COMPRESS_RGTC_RG;
		else
			internalFormat = TEX_COMPRESS_NONE;
		break;
	case 1:
		format = TEXFMT_RED;
		if (compress)
			internalFormat = TEX_COMPRESS_RGTC_RED;
		else
			internalFormat = TEX_COMPRESS_NONE;
		break;
	default:
		plog::error("In Texture::LoadImage():\n");
		plog::errmore("Error while loading '%s':\n", filename);
		plog::errmore("Invalid number of channels.\n");
		stbi_image_free(image);
		LoadDebug();
		return 0;
	}

	LoadPixels(w, h, internalFormat, format, TEXTYPE_UBYTE, image);
	setFiltering(TEXFILTER_LINEAR, TEXFILTER_LINEAR);
	setWrapping(TEXWRAP_REPEAT, TEXWRAP_REPEAT);

	stbi_image_free(image);
	return channels;
}

unsigned int Texture::LoadCubemapImages(const char* filenames[], bool srgb, bool compress, bool rgba1bit)
{
	int w[6] = {};
	int h[6] = {};
	int channels[6] = {};
	unsigned char* image[6] = {};

	// Open the file.
	for (int i = 0; i < 6; ++i)
	{
		InFile file = filemanager::LoadSingleFile(filenames[i], std::ios::binary);
		std::string contents = file.contents();

		// Load the image.
		image[i] = stbi_load_from_memory((unsigned char*)contents.c_str(), (int)contents.size(), &w[i], &h[i], &channels[i], 0);
		if (!image[i])
		{
			plog::error("In Texture::LoadCubemapImages():\n");
			plog::errmore("Couldn't load '%s':\n%s\n", filenames[i], stbi_failure_reason());
//			LoadDebug();
			return 0;
		}
	}

	// Make sure the dimensions and format are the same for all 6 images.
	for (int i = 1; i < 6; ++i)
	{
		if (w[0] != w[i] || h[0] != h[i] || channels[0] != channels[i])
		{
			plog::error("In Texture::LoadCubemapImages():\n");
			plog::errmore("All 6 images of a cubemap must have the same format and dimensions.\n");

			for (int j = 0; j < 6; ++j)
			{
				stbi_image_free(image[j]);
			}
			return 0;
		}
	}

	// Figure out the pixel format //
	TexEnum format;
	TexEnum internalFormat;
	switch (channels[0])
	{
	case 4:
		if (srgb)
		{
			format = TEXFMT_SRGBA;
			if (compress)
			{
				if (rgba1bit)
					internalFormat = TEX_COMPRESS_S3TC_SRGBA_DXT1;
				else
					internalFormat = TEX_COMPRESS_S3TC_SRGBA_DXT5;
			}
			else
				internalFormat = TEX_COMPRESS_NONE;
		}
		else
		{
			format = TEXFMT_RGBA;
			if (compress)
			{
				if (rgba1bit)
					internalFormat = TEX_COMPRESS_S3TC_RGBA_DXT1;
				else
					internalFormat = TEX_COMPRESS_S3TC_RGBA_DXT5;
			}
			else
				internalFormat = TEX_COMPRESS_NONE;
		}
		break;
	case 3:
		if (srgb)
		{
			format = TEXFMT_SRGB;
			if (compress)
				internalFormat = TEX_COMPRESS_S3TC_SRGB_DXT1;
			else
				internalFormat = TEX_COMPRESS_NONE;
		}
		else
		{
			format = TEXFMT_RGB;
			if (compress)
				internalFormat = TEX_COMPRESS_S3TC_RGB_DXT1;
			else
				internalFormat = TEX_COMPRESS_NONE;
		}
		break;
	case 2:
		format = TEXFMT_RG;
		if (compress)
			internalFormat = TEX_COMPRESS_RGTC_RG;
		else
			internalFormat = TEX_COMPRESS_NONE;
		break;
	case 1:
		format = TEXFMT_RED;
		if (compress)
			internalFormat = TEX_COMPRESS_RGTC_RED;
		else
			internalFormat = TEX_COMPRESS_NONE;
		break;
	default:
		plog::error("In Texture::LoadCubemapImages():\n");
		plog::errmore("Invalid number of channels.\n");
		for (int i = 0; i < 6; ++i)
		{
			stbi_image_free(image[i]);
		}
//		LoadDebug();
		return 0;
	}

	LoadCubemapPixels(w[0], h[0], internalFormat, format, TEXTYPE_UBYTE, (void**)image);

	for (int i = 0; i < 6; ++i)
	{
		stbi_image_free(image[i]);
	}
	return channels[0];
}

void Texture::LoadDebug()
{
	unsigned char pixels[] =
	{
		255, 0, 255, 255,
		0, 255, 255, 255,
		0, 255, 255, 255,
		255, 0, 255, 255,
	};

	LoadPixels(2, 2, TEX_COMPRESS_NONE, TEXFMT_SRGBA, TEXTYPE_UBYTE, pixels);
	setFiltering(TEXFILTER_NEAREST, TEXFILTER_NEAREST);
	setWrapping(TEXWRAP_REPEAT, TEXWRAP_REPEAT);
}

void Texture::UseDebug(unsigned int index)
{
	static Texture debug_tex;

	if (!debug_tex.isReady())
	{
		debug_tex.LoadDebug();
	}

	debug_tex.Use2D(index);
}

void Texture::UseBlack(unsigned int index)
{
	static Texture black_tex;

	if (!black_tex.isReady())
	{
		unsigned char pixels[] =
		{
			0, 0, 0
		};

		black_tex.LoadPixels(1, 1, TEX_COMPRESS_NONE, TEXFMT_RGB, TEXTYPE_UBYTE, pixels);
		black_tex.setFiltering(TEXFILTER_NEAREST, TEXFILTER_NEAREST);
	}

	black_tex.Use2D(index);
}

void Texture::UseWhite(unsigned int index)
{
	static Texture white_tex;

	if (!white_tex.isReady())
	{
		unsigned char pixels[] =
		{
			255, 255, 255
		};

		white_tex.LoadPixels(1, 1, TEX_COMPRESS_NONE, TEXFMT_RGB, TEXTYPE_UBYTE, pixels);
		white_tex.setFiltering(TEXFILTER_NEAREST, TEXFILTER_NEAREST);
	}

	white_tex.Use2D(index);
}

void Texture::UseBlankNormal(unsigned int index)
{
	static Texture normal_tex;

	if (!normal_tex.isReady())
	{
		unsigned char pixels[] =
		{
			127, 127, 255
		};

		normal_tex.LoadPixels(1, 1, TEX_COMPRESS_NONE, TEXFMT_RGB, TEXTYPE_UBYTE, pixels);
		normal_tex.setFiltering(TEXFILTER_NEAREST, TEXFILTER_NEAREST);
	}

	normal_tex.Use2D(index);
}

void Texture::UseRandom(unsigned int index)
{
	static Texture random_tex;

	if (!random_tex.isReady())
	{
		const int width = 64;
		const int height = 64;
		unsigned char pixels[width][height][2];
		srand(1);

		for (int i = 0; i < width; ++i)
		{
			for (int j = 0; j < height; ++j)
			{
				// Get a random value between 0 and 1
				float val = (float)rand() / (float)RAND_MAX;

				// Convert that to a random radian angle between 0 and TAU
				float angle = val * TAU;

				// Get the sine and cosine of the random angle
				float sin_angle = sin(angle);
				float cos_angle = cos(angle);

				// Convert the values from [-1,1] to [0,1]
				sin_angle = (sin_angle * 0.5f) + 0.5f;
				cos_angle = (cos_angle * 0.5f) + 0.5f;

				// Save the values in the texture (0 to 255)
				pixels[i][j][0] = (unsigned char)(sin_angle * 255.0f);
				pixels[i][j][1] = (unsigned char)(cos_angle * 255.0f);
			}
		}

		random_tex.LoadPixels(width, height, TEX_COMPRESS_NONE, TEXFMT_RG, TEXTYPE_UBYTE, pixels);
		random_tex.setFiltering(TEXFILTER_NEAREST, TEXFILTER_NEAREST);
		random_tex.setWrapping(TEXWRAP_REPEAT, TEXWRAP_REPEAT);
	}

	random_tex.Use2D(index);
}