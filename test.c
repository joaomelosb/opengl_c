#include <stdio.h>

typedef union {
	struct {
		float x, y, z;
	};
	struct {
		float r, g, b;
	};
	float raw[3];
} vec3s;

int main(int argc, char **argv) {

	vec3s vec = {.x = 0, .y = 1, .z = 2};

	printf("%f %f %f\n", *vec.raw, vec.raw[1],
		vec.raw[2]);

	return 0;
}
