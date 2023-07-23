#include <math.h>
#include <errno.h>
#include <string.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "cglm/struct.h"
#include "keyboard.h"
#include "texture.h"
#include "shader.h"
#include "common.h"
#include "log.h"

static SDL_Window *window;
static SDL_GLContext context;

#undef main

int main(void) {

	SDL_TEST(!SDL_Init(SDL_INIT_VIDEO),
		"init video");

	SDL_TEST(window = SDL_CreateWindow(
		"Cube",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		500, 500,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
	), "create window");

	SDL_TEST(context = SDL_GL_CreateContext(window),
		"create context");

	GLenum glew;

	TEST((glew = glewInit()) == GLEW_OK,
		"glew init failed: %s", glewGetErrorString(glew));

	if (SDL_GL_SetSwapInterval(1) && SDL_GL_SetSwapInterval(-1))
		LOG("warning: vsync not supported!");

	GLuint program = glCreateProgram();

	if (!attachShaderFile(program, GL_VERTEX_SHADER, "shaders/cube.vs") ||
		!attachShaderFile(program, GL_FRAGMENT_SHADER, "shaders/cube.fs") ||
		!linkProgram(program))
		return 1;

	glUseProgram(program);

#define VBO *buffers
#define EBO buffers[1]
#define INDEX(off) off, off + 1, off + 2, \
	off, off + 2, off + 3

	GLuint VAO, buffers[2];

	struct {
		vec3 vertex;
		vec3 normal;
		vec2 tex;
	} const vertex[] = {
		// front
		{{-1, +1, -1}, {+0, +0, -1}, {0, 0}},
		{{+1, +1, -1}, {+0, +0, -1}, {1, 0}},
		{{+1, -1, -1}, {+0, +0, -1}, {1, 1}},
		{{-1, -1, -1}, {+0, +0, -1}, {0, 1}},

		// top
		{{-1, +1, +1}, {+0, +1, +0}, {0, 0}},
		{{+1, +1, +1}, {+0, +1, +0}, {1, 0}},
		{{+1, +1, -1}, {+0, +1, +0}, {1, 1}},
		{{-1, +1, -1}, {+0, +1, +0}, {0, 1}},

		// back
		{{-1, +1, +1}, {+0, +0, +1}, {0, 0}},
		{{+1, +1, +1}, {+0, +0, +1}, {1, 0}},
		{{+1, -1, +1}, {+0, +0, +1}, {1, 1}},
		{{-1, -1, +1}, {+0, +0, +1}, {0, 1}},

		// bottom
		{{-1, -1, +1}, {+0, -1, +0}, {0, 0}},
		{{+1, -1, +1}, {+0, -1, +0}, {1, 0}},
		{{+1, -1, -1}, {+0, -1, +0}, {1, 1}},
		{{-1, -1, -1}, {+0, -1, +0}, {0, 1}},

		// left
		{{-1, +1, +1}, {-1, +0, +0}, {0, 0}},
		{{-1, +1, -1}, {-1, +0, +0}, {1, 0}},
		{{-1, -1, -1}, {-1, +0, +0}, {1, 1}},
		{{-1, -1, +1}, {-1, +0, +0}, {0, 1}},

		// right
		{{+1, +1, -1}, {+1, +0, +0}, {0, 0}},
		{{+1, +1, +1}, {+1, +0, +0}, {1, 0}},
		{{+1, -1, +1}, {+1, +0, +0}, {1, 1}},
		{{+1, -1, -1}, {+1, +0, +0}, {0, 1}}		
	};

	GLubyte const indices[] = {
		INDEX(0),
		INDEX(4),
		INDEX(8),
		INDEX(12),
		INDEX(16),
		INDEX(20)
	};

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(2, buffers);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertex, vertex,
		GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof *vertex, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof *vertex, (void *)sizeof vertex->vertex);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof *vertex, (void *)(sizeof vertex->vertex + sizeof vertex->normal));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices,
		GL_STATIC_DRAW);

	// setup texture
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	if (!load_texture_2d("textures/diamond.jpg"))
		LOG("warning: can't load texture: %s", strerror(errno));

	// setup depth test
	glEnable(GL_DEPTH_TEST);

	mat4 mat;

	struct {
		vec3s pos;
		vec3s dir;
		vec3s up;
	} cam = {
		{
			.x = 0,
			.y = 0,
			.z = -10
		},
		{
			.x = 0,
			.y = 0,
			.z = 1
		},
		{
			.x = 0,
			.y = 1,
			.z = 0
		}
	};

	GLint
		m = glGetUniformLocation(program, "m"),
		v = glGetUniformLocation(program, "v"),
		p = glGetUniformLocation(program, "p");

	SDL_bool relative = SDL_FALSE;
	float yaw = 3.14 / 2, pitch = 0;

	key_init();

	for (Uint64 lastTime = 0;;) {

		SDL_Event e;

		while (SDL_PollEvent(&e))
			if (e.type == SDL_QUIT)
				return 0;

		int w, h;

		SDL_GetWindowSize(window, &w, &h);

		glm_perspective(3.14 / 4, (float)w / h,
			1, 100, mat);

		glUniformMatrix4fv(
			p,
			1,
			GL_FALSE,
			*mat
		);

		// Now, setup objects of the scene
		Uint64 curTime = SDL_GetTicks64();

		if (!lastTime)
			lastTime = curTime;

		float dt = (curTime - lastTime) / 1000.f;
		lastTime = curTime;

#define DIR(a, b) (key_pressed(KEY(a)) - key_pressed(KEY(b)))
#define SPEED 10

		if (key_once_pressed(KEY(F)))
			SDL_SetRelativeMouseMode(relative ^= SDL_TRUE);

		if (relative) {
			int xrel, yrel;

			SDL_GetRelativeMouseState(&xrel, &yrel);

			yaw += xrel / 10. * dt;
			pitch += yrel / -10. * dt;
		}

		cam.dir.x = cosf(yaw) * cosf(pitch);
		cam.dir.y = sinf(pitch);
		cam.dir.z = sinf(yaw) * cosf(pitch);

		cam.pos.x += DIR(A, D) * dt * SPEED;
		cam.pos.y += DIR(SPACE, LSHIFT) * dt * SPEED;
		cam.pos.z += DIR(W, S) * dt * SPEED;

		glm_look(cam.pos.raw, cam.dir.raw, cam.up.raw, mat);

		glUniformMatrix4fv(
			v,
			1,
			GL_FALSE,
			*mat
		);	

		glClearColor(.2, .2, .2, 1);
		glClear(GL_COLOR_BUFFER_BIT |
			GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, w, h);

#define W 20
#define H 10

		vec3 tmp = {[2] = 0};

		for (int i = 0; i < H; i++) {
			tmp[1] = (H - i - 1) * 2;
			for (int j = 0; j < W; j++) {
				*tmp = (W / -2 + j) * 2;
				glm_translate_make(mat, tmp);
				glUniformMatrix4fv(
					m,
					1,
					GL_FALSE,
					*mat
				);
				glDrawElements(GL_TRIANGLES,
					sizeof indices / sizeof *indices,
						GL_UNSIGNED_BYTE, 0);
			}
		}

		SDL_GL_SwapWindow(window);

	}

}
