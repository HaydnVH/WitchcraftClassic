#ifndef HVH_WC_GRAPHICS_TEXTURE_H
#define HVH_WC_GRAPHICS_TEXTURE_H

#include <stdint.h>
#include <string.h>

enum TexEnum
{
	TEXFMT_RED = 1,	// 1-channel "red" texture
	TEXFMT_RG,		// 2-channel "red/green" texture
	TEXFMT_RGB,		// 3-channel "red/green/blue" texture
	TEXFMT_SRGB,	// 3-channel "red/green/blue" texture in SRGB color space
	TEXFMT_RGBA,	// 4-channel "red/green/blue/alpha" texture
	TEXFMT_SRGBA,	// 4-channel "red/green/blue/alpha" texture in SRGB color space (linear alpha)

	TEX_COMPRESS_NONE = 0,			// Uncompressed
	TEX_COMPRESS_S3TC_RGB_DXT1,		// 3-channel S3TC
	TEX_COMPRESS_S3TC_SRGB_DXT1,	// 3-channel S3TC in SRGB color space
	TEX_COMPRESS_S3TC_RGBA_DXT1,	// 4-channel S3TC, with 1-bit alpha
	TEX_COMPRESS_S3TC_SRGBA_DXT1,	// 4-channel S3TC in SRGB color space, with 1-bit alpha
	TEX_COMPRESS_S3TC_RGBA_DXT5,	// 4-channel S3TC, with full alpha
	TEX_COMPRESS_S3TC_SRGBA_DXT5,	// 4-channel S3TC in SRGB color space, with full linear alpha
	TEX_COMPRESS_RGTC_RED,			// 1-channel RGTC
	TEX_COMPRESS_RGTC_RG,			// 2-channel RGTC

	TEXTYPE_BYTE = 0,
	TEXTYPE_UBYTE,
	TEXTUPE_SHORT,
	TEXTYPE_USHORT,
	TEXTYPE_INT,
	TEXTYPE_UINT,
	TEXTYPE_FLOAT,

	TEXWRAP_CLAMP = 0,
	TEXWRAP_REPEAT,

	TEXFILTER_NEAREST = 0,
	TEXFILTER_LINEAR,
};

class Texture
{
public:
	Texture()
	{
		memset(this, 0, sizeof(Texture));
	}
	Texture(const Texture& rhs) = delete;
	Texture(Texture&& rhs)
	{
		// Move constructor.
		memcpy(this, &rhs, sizeof(Texture)); // copy rhs into this.
		memset(&rhs, 0, sizeof(Texture)); // set rhs to 0s (so its destructor doesn't do anything dangerous).
	}
	inline Texture& operator = (const Texture& rhs) = delete;
	inline Texture& operator = (Texture&& rhs)
	{
		// Move-assignment operator, this is just copy-and-swap.
		Texture temp; // temporary storage
		memcpy(&temp, this, sizeof(Texture)); // copy this into temp
		memcpy(this, &rhs, sizeof(Texture)); // copy rhs into this
		memcpy(&rhs, &temp, sizeof(Texture)); // copy temp into rhs
		memset(&temp, 0, sizeof(Texture)); // set temp to 0s
		return *this;
	}
	~Texture()
		{ Clean(); };

	void Clean();

	unsigned int LoadImage(const char* filename, bool srgb = false, bool compress = false, bool rgba1bit = false);
	unsigned int LoadCubemapImages(const char* filenames[], bool srgb = false, bool compress = false, bool rgba1bit = false);

	void LoadDebug();
	void LoadBlank();
	void LoadBlankNormal();

	static void UseDebug(unsigned int index);
	static void UseBlack(unsigned int index);
	static void UseWhite(unsigned int index);
	static void UseBlankNormal(unsigned int index);
	static void UseRandom(unsigned int index);

	void LoadPixels(unsigned int width, unsigned int height, TexEnum compression, TexEnum format, TexEnum type, void* pixels);
	void LoadSubPixels(int xoffset, int yoffset, unsigned int width, unsigned int height, TexEnum format, TexEnum type, void* pixels);

	void LoadCubemapPixels(unsigned int width, unsigned int height, TexEnum compression, TexEnum format, TexEnum type, void* pixels[]);

	void GenerateMipmaps();

	bool Use2D(unsigned int index);
	bool UseCube(unsigned int index);

	void setFiltering(TexEnum min, TexEnum mag, float aniso = 0.0f);
	void setWrapping(TexEnum s, TexEnum t);

	bool isReady();

private:

	bool isCubemap;

#ifdef RENDERER_OPENGL
	uint32_t tex;
#elif RENDERER_VULKAN
#elif RENDERER_DIRECTX
#endif
};

#endif