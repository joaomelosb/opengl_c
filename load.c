#include <stdlib.h>
#include "config.h"
#include "load.h"
#include "log.h"

// read LOAD_BLOCKs into heap allocated buffer
// returns NULL if could not open or read the file
// free returned buffer to avoid leaks :)
char *load(char const *file) {

	FILE *fp = fopen(file, "rb");

	if (!fp)
		return NULL;

	char *buf = 0;
	size_t count = 0;
	int err;

	do {
		buf = realloc(buf, count * sizeof *buf +
			LOAD_BLOCK * sizeof *buf);
		count += fread(
			buf + count,
			1,
			LOAD_BLOCK,
			fp
		);
	} while (!(err = ferror(fp)) && !feof(fp));

	if (err) {
		free(buf);
		return NULL;
	}

	LOG("\"%s\": loaded %zu characters", file, count);

	buf[count] = 0;

	return buf;

}
