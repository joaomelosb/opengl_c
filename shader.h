#ifndef SHADER_H
#define SHADER_H

#include <stdbool.h>
#include <GL/glew.h>
#include "decl.h"

BEGIN_DECL

bool attachShaderFile(GLuint, GLenum, char const *);

bool linkProgram(GLuint);

END_DECL

#endif
