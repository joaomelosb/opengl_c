#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "cglm/struct.h"
#include "shader.h"
#include "log.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "common.h"

#undef main

static SDL_Window *window;
static SDL_GLContext context;

int main() {

	SDL_TEST(!SDL_Init(SDL_INIT_VIDEO), "init video");

	SDL_TEST(window = SDL_CreateWindow(
		"Foo window",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		500, 500,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
	), "create window");

	SDL_TEST(context = SDL_GL_CreateContext(window),
		"create context");

	GLenum glew;

	TEST((glew = glewInit()) == GLEW_OK,
		"failed to init glew: %s", glewGetErrorString(glew));

	if (SDL_GL_SetSwapInterval(-1) && SDL_GL_SetSwapInterval(1))
		LOG("VSYNC unsupported");

	GLuint program = glCreateProgram();

	if (!attachShaderFile(program, GL_VERTEX_SHADER, "shaders/vertex.glsl") ||
		!attachShaderFile(program, GL_FRAGMENT_SHADER, "shaders/frag.glsl") ||
		!linkProgram(program))
		return 1;

	glUseProgram(program);

	GLuint VAO, VBO, EBO;

#define IW 1233.
#define IH 100.
#define X (848 / IW)
#define Y (2 / IH)
#define W (44 / IW)
#define H (47 / IH)

	GLfloat const vertex[][4] = {
		{-.5, +.5, X, Y + H},
		{+.5, +.5, X + W, Y + H},
		{+.5, -.5, X + W, Y},
		{-.5, -.5, X, Y}
	};

	GLubyte const indices[] = {
		0, 1, 2,
		0, 2, 3
	};

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertex, vertex, GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof *vertex, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof *vertex, (void *)(sizeof (float) * 2));
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices,
		GL_STATIC_DRAW);

	// Setup texture
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

#define TEXTURE "textures/dino.png"

	int iw, ih, in;
	unsigned char *buf;

	if ((buf = stbi_load(TEXTURE, &iw, &ih, &in, 0))) {
		GLenum type =
			in == 1 ? GL_RED :
			in == 2 ? GL_RG :
			in == 3 ? GL_RGB :
			GL_RGBA;
		glTexImage2D(GL_TEXTURE_2D, 0, type, iw, ih, 0, type,
			GL_UNSIGNED_BYTE, buf);
		stbi_image_free(buf);
	} else
		LOG_E("failed to load \"" TEXTURE "\": %s", strerror(errno));

	// Setup depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GEQUAL);

	GLint
		transform = glGetUniformLocation(program, "transform"),
		frameSize = glGetUniformLocation(program, "frame.size"),
		framePos = glGetUniformLocation(program, "frame.pos"),
		frameBase = glGetUniformLocation(program, "frame.base");

	glUniform2f(frameSize, W, H);

#define GRAVITY 1
#define JUMPSTRENGTH 30
#define FPS 10

	mat4 mat;

	vec3s
		siz = {.raw = {0}, .x = 300, .y = 300},
		pos = {.raw = {0}, .x = siz.x / 2, .y = siz.y / 2};

	vec2s
		spd = {.x = 0, .y = 0};

	ivec2s
		frame = {.raw = {0}};

	int frameLen = 1;

	Uint64 lastTime = 0;

	for (;;) {

		SDL_Event e;

		while (SDL_PollEvent(&e))
			if (e.type == SDL_QUIT) {
				LOG("SDL_QUIT, exit!");
				return 0;
			}

		int w, h;

		SDL_GetWindowSize(window, &w, &h);

		glm_ortho(0, w, h, 0, 0, 1, mat);
		glm_translate(mat, pos.raw);
		glm_scale(mat, siz.raw);

#define DIR(l, r, spd) (kb[SDL_SCANCODE_##r] - kb[SDL_SCANCODE_##l]) * (spd)

		Uint8 const *kb = SDL_GetKeyboardState(0);

		if (kb[SDL_SCANCODE_SPACE] &&
			pos.y == h - siz.y / 2)
			spd.y = -JUMPSTRENGTH;
		else if (!kb[SDL_SCANCODE_SPACE] &&
			spd.y < -JUMPSTRENGTH / 2)
			spd.y = -JUMPSTRENGTH / 2;

		spd.x = DIR(LEFT, RIGHT, 5);

		if ((spd.x < 0 && siz.x > 0) ||
			(spd.x > 0 && siz.x < 0))
			siz.x = -siz.x;

		pos.x += spd.x;
		pos.y += spd.y += GRAVITY;

		float _floor = h - siz.y / 2;

		if (pos.y > _floor) {
			pos.y = _floor;
			spd.y = 0;
		}

		Uint64 curTime = SDL_GetTicks64();

#define SET_ANIM(len, bx, by) do { \
	glUniform2i(frameBase, bx, by); \
	lastTime = curTime; \
	frameLen = len; \
	frame.x = 0; \
} while (0)

		if (pos.y == _floor &&
			spd.x && frameLen == 1)
			SET_ANIM(2, 2, 0);
		else if ((!spd.x || pos.y < _floor)
			&& frameLen == 2)
			SET_ANIM(1, 0, 0);

		if (!lastTime)
			lastTime = curTime;

		if (curTime - lastTime >= 1000. / FPS) {
			if (++frame.x == frameLen)
				frame.x = 0;
			lastTime = curTime;
		}

		glUniform2iv(framePos, 1, frame.raw);

		glUniformMatrix4fv(
			transform,
			1,
			GL_FALSE,
			*mat
		);

		glViewport(0, 0, w, h);
		glClearColor(.2, .2, .2, 1);
		glClear(GL_COLOR_BUFFER_BIT |
			GL_DEPTH_BUFFER_BIT);
		glDrawElements(GL_TRIANGLES, sizeof indices / sizeof *indices,
			GL_UNSIGNED_BYTE, 0);

		SDL_GL_SwapWindow(window);

	}

	return 0;

}
