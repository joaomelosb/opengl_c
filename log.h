#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#define _LOG(file, ...) do { \
	fprintf(file, __FILE__ ":%s(%d): ", __func__, __LINE__); \
	fprintf(file, __VA_ARGS__); \
	fputc('\n', file); \
} while (0)

#define LOG(...) _LOG(stdout, __VA_ARGS__)
#define LOG_E(...) _LOG(stderr, __VA_ARGS__)

#define GL_DEBUG() do { \
	GLenum error = glGetError(); \
	LOG("glGetError(): %#x", error); \
} while (0)

#endif
