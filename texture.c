#include <GL/glew.h>
#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// load pixel on GL_TEXTURE_2D target
bool load_texture_2d(char const *path) {

	unsigned char *buf;
	int w, h, n;

	if (!(buf = stbi_load(path, &w, &h, &n, 0)))
		return false;

	GLint type =
		n == 2 ? GL_RG :
		n == 3 ? GL_RGB :
		n == 4 ? GL_RGBA :
		GL_RED;

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		type,
		w, h,
		0,
		type,
		GL_UNSIGNED_BYTE,
		buf
	);

	stbi_image_free(buf);

	return true;

}
