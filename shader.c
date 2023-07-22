#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "shader.h"
#include "load.h"
#include "log.h"

static char *checkGLObj(
	GLuint obj,
	GLenum param,
	void (GLAPIENTRY *getiv)(GLuint, GLenum, GLint *),
	void (GLAPIENTRY *getil)(GLuint, GLsizei, GLsizei *, GLchar *)) {

	GLint data;

	getiv(obj, param, &data);

	if (!data) {
		getiv(obj, GL_INFO_LOG_LENGTH, &data);
		char *log = malloc(data);
		getil(obj, data, 0, log);
		return log;
	}

	return 0;

}

bool attachShaderFile(GLuint program, GLenum type,
	char const *file) {

	char *src;

	if (!(src = load(file))) {
		LOG_E("could not read \"%s\": %s", file, strerror(errno));
		return false;
	}

	GLuint shader = glCreateShader(type);

	glShaderSource(shader, 1, (GLchar const **)&src, 0);
	glCompileShader(shader);
	free(src);

	char *log;

	if ((log = checkGLObj(shader, GL_COMPILE_STATUS,
		glGetShaderiv, glGetShaderInfoLog))) {
		LOG_E("couldn't compile \"%s\" shader: %s", file, log);
		glDeleteShader(shader);
		free(log);
		return false;
	}

	glAttachShader(program, shader);
	glDeleteShader(shader);

	return true;

}

bool linkProgram(GLuint program) {

	glLinkProgram(program);

	char *log;

	if ((log = checkGLObj(program, GL_LINK_STATUS,
		glGetProgramiv, glGetProgramInfoLog))) {
		LOG_E("failed to compile program: %s", log);
		free(log);
		return false;
	}

	return true;

}
