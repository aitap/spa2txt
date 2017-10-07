#include <stddef.h>

enum spa_parse_result {
	spa_ok,
	spa_open_error,
	spa_seek_error,
	spa_read_error,
	spa_alloc_error
};
enum spa_parse_result spa_parse(
	/* in */ const char * /* filename */,
	/* out, opt */ char /* comment */ [256],
	/* out */ uint32_t * /* num_points */,
	/* out */ float ** /* wavelengths */,
	/* out */ float ** /* intensities */
);
