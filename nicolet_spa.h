#include <stddef.h>
char spa_parse(
	/* in */ const char * /* filename */,
	/* out, opt */ char /* comment */ [256],
	/* out */ size_t * /* num_points */,
	/* out */ float ** /* wavelengths */,
	/* out */ float ** /* intensities */
);
