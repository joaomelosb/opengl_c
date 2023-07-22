#ifndef COMMON_H
#define COMMON_H

#define TEST(expr, ...) do { \
	if (!(expr)) { \
		LOG_E(__VA_ARGS__); \
		return 1; \
	} \
} while (0)

#define SDL_TEST(expr, s) TEST(expr, s " failed: %s", SDL_GetError())

#endif /* COMMON_H */
