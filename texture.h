#pragma once


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


struct Texture {
	int width;
	int height;
	int channels;
	int size;

	static Texture load(const char* filename) {
		Texture texture;

		texture.stbPixels = stbi_load(
				filename,
				&texture.width,
				&texture.height,
				&texture.channels,
				STBI_rgb_alpha); // force the image to be loaded with an alpha channel (4 bytes per pixel)

		if (!texture.stbPixels) {
			throw std::runtime_error("failed to load texture image");
		}

		texture.size = texture.width * texture.height * 4;

		return texture;
	}

  unsigned char* getPixels() {
    return static_cast<unsigned char*>(stbPixels);
  }

  ~Texture() {
    stbi_image_free(stbPixels);
  }


 private:
	stbi_uc* stbPixels;

  Texture() {}
};
